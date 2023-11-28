//-----------------------shop.cpp----------------------------
//Kelly Kauffman
//CSS 503
//Date created 5.1.23
//Date last edited 5.7.23

//Description: implementation file for the Shop class
//Includes methods for the barber and customer actions, in addition
//to helper methods to print output to the screen.

//------------------------------------------------------------

#include "shop.h"

//init()
//initilizes mutex, condition variables, and all arrays
void Shop::init(int num_barbers) {
  pthread_mutex_init(&mutex_, NULL);
  pthread_cond_init(&cond_customers_waiting_, NULL);

  for (int i = 0; i < num_barbers; i++) {
    pthread_cond_init(&cond_customer_served_[i], NULL);
    pthread_cond_init(&cond_barber_paid_[i], NULL);
    pthread_cond_init(&cond_barber_sleeping_[i], NULL);

    in_service_[i] = false;
    money_paid_[i] = false;
    customer_in_chair_[i] = 0; //0 means the barber's chair is empty
  }

}

//int2string()
//changes an integer to a string
string Shop::int2string(int i) {
  stringstream out;
  out << i;
  return out.str();
}

//print()
//prints out the step a customer or barber is on
void Shop::print(int person, bool type, string message) {
  //the boolean type will determine if the message needs to start
  //with customer or barber
  cout << ((type == true) ? "customer[" : "barber  [") << person
       << "]: " << message << endl;
}

//get_cust_drops()
//returns the count of customers that did not get service
int Shop::get_cust_drops() const { return cust_drops_; }

//visitShop()
//a customer thread calls this function to enter the shop
//the customer may leave if waiting chairs are full
//or gets paired with an available barber
int Shop::visitShop(int id) // int id is the customer id
{
  int barber_;  //the barber the customer will get paired with
  bool allChairsFull = true;  //check if barber chairs are full
  
//enter critical section
  pthread_mutex_lock(&mutex_);

  //check to see if all barber chairs are full
  for (int i = 0; i < barber_count_; i++) {
    if(customer_in_chair_[i] == 0) {
      //stop checking for available chairs
      allChairsFull = false;
      break;
    }
  }

    // If all barber chairs are full and no waiting chairs then leave shop
  if (allChairsFull && max_waiting_cust_ == 0) {
    print(id, true, "leaves the shop because of no available waiting chairs.");
    ++cust_drops_;
    pthread_mutex_unlock(&mutex_);
    return -1;
  }
  
  // If all barber and waiting chairs are full then leave shop
  if (allChairsFull && waiting_chairs_.size() == max_waiting_cust_) {
    print(id, true, "leaves the shop because of no available waiting chairs.");
    ++cust_drops_;
    pthread_mutex_unlock(&mutex_);
    return -1;
  }
  
  // If someone is being served or transitioning waiting to service chair
  // then take a chair and wait for service
  if (allChairsFull || !waiting_chairs_.empty()) {
    waiting_chairs_.push(id);
    print(id, true, "takes a waiting chair. # waiting seats available = " +
                  int2string(max_waiting_cust_ - waiting_chairs_.size()));
    pthread_cond_wait(&cond_customers_waiting_, &mutex_);
    waiting_chairs_.pop();
  }

  //find the available chair
  for (int i = 0; i < barber_count_; i++) {
    //if a chair is available, then get assigned to that barber
    //stop checking for available chairs
    if(customer_in_chair_[i] == 0) {
      barber_ = i;
      break;
    }
  }
  
  print(id, true, "moves to the service chair [" + int2string(barber_) + "]. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
  customer_in_chair_[barber_] = id;  //fill barber's chair
  in_service_[barber_] = true;

  // wake up the barber just in case if he is sleeping
  pthread_cond_signal(&cond_barber_sleeping_[barber_]);

  //leave critical section
  pthread_mutex_unlock(&mutex_);
  
  return barber_; 
}

//leaveShop()
//a customer thread calls this function when getting a haircut
//customer will pay and leave the shop
void Shop::leaveShop(int id, int barber_id) {

  //enter critical section
  pthread_mutex_lock(&mutex_);

  // Wait for service to be completed
  print(id, true, "wait for barber[" + int2string(barber_id) + "] to be done with the hair-cut");
  while (in_service_[barber_id] == true) {
      pthread_cond_wait(&cond_customer_served_[barber_id], &mutex_);
  }

  // Pay the barber and signal barber appropriately
  money_paid_[barber_id] = true;
  pthread_cond_signal(&cond_barber_paid_[barber_id]);
  print(id, true, "says good-bye to barber [" + int2string(barber_id) + "].");
  
  //leave critical section
  pthread_mutex_unlock(&mutex_);
}

//helloCustomer()
//a barber thread calls this function to check for customers
//and start a haircut service
void Shop::helloCustomer(int id) {

  //enter critical section
  pthread_mutex_lock(&mutex_);

  // If no customers than barber can sleep
  if (waiting_chairs_.empty() && customer_in_chair_[id] == 0) {
    print(id, false, "sleeps because of no customers.");
    pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
  }

  if (customer_in_chair_[id] == 0) // check if the customer, sleep if chair isn't taken
  {
    pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
  }

  //start a haircut service
  print(id, false,
        "starts a hair-cut service for customer[" + int2string(customer_in_chair_[id]) + "]");

  //leave critical section
  pthread_mutex_unlock(&mutex_);
}

//byeCustomer()
//a barber thread calls this function to finish a haircut service
void Shop::byeCustomer(int id) {
  
  //enter critical section
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_[id] = false;
  print(id, false, "says he's done with a hair-cut service for customer[" +
                    int2string(customer_in_chair_[id]) + "]");
  money_paid_[id] = false;
  pthread_cond_signal(&cond_customer_served_[id]);

  while (money_paid_[id] == false) {
    pthread_cond_wait(&cond_barber_paid_[id], &mutex_);
  }

  // Signal to customers waiting to get next one
  customer_in_chair_[id] = 0;
  print(id, false, "calls in another customer");
  pthread_cond_signal(&cond_customers_waiting_);

  //leave critical section
  pthread_mutex_unlock(&mutex_); 
}
