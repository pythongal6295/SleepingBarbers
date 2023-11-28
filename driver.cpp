//-----------------------driver.cpp----------------------------
//Kelly Kauffman
//CSS 503
//Date created 5.1.23
//Date last edited 5.7.23

//Description: driver file to test the sleeping barbers problem
//Includes ThreadParam class to implement barber and customer threads.
//Barber and customer threads are created in this file.

//------------------------------------------------------------

#include "shop.h"
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

void *barber(void *);
void *customer(void *);

// ThreadParam class
// This class is used as a way to pass more
// than one argument to a thread.
class ThreadParam {
public:
  ThreadParam(Shop *shop, int id, int service_time)
      : shop(shop), id(id), service_time(service_time){};
  Shop *shop;       // pointer to Shop object
  int id;           // a thread identifier
  int service_time; // service time (usec) for barber; 0 for customer
};

int main(int argc, char *argv[]) {

  // Read arguments from command line
  // Validate values
  if (argc != 5) {
    cout << "Usage: num_barbers num_chairs num_customers service_time" << endl;
    return -1;
  }
  int num_barbers = atoi(argv[1]);
  int num_chairs = atoi(argv[2]);
  int num_customers = atoi(argv[3]);
  int service_time = atoi(argv[4]);

  // many barbers, one shop, many customers
  pthread_t barber_threads[num_barbers];
  pthread_t customer_threads[num_customers];
  Shop shop(num_barbers, num_chairs);
  

  //for loop to create new barber threads with a specific id
  for (int i = 0; i < num_barbers; i++) {
    int id = i;
    ThreadParam *barber_param = new ThreadParam(&shop, id, service_time);
    pthread_create(&barber_threads[i], NULL, barber, barber_param);
  }

  //for loop to create new customer threads with a specific id and random waiting time
  for (int i = 0; i < num_customers; i++) {
    usleep(rand() % 1000);
    int id = i + 1;
    ThreadParam *customer_param = new ThreadParam(&shop, id, 0);
    pthread_create(&customer_threads[i], NULL, customer, customer_param);
  }

  // Wait for customers to finish
  for (int i = 0; i < num_customers; i++) {
    pthread_join(customer_threads[i], NULL);
  }

  //Check if barber threads are canceled correctly
  int cancelCheck = 0;
  
  for (int i = 0; i < num_barbers; i++) {
    cancelCheck = pthread_cancel(barber_threads[i]);

    if (cancelCheck != 0) {
      cout << "Thread cancelling failed for barber thread " << i + 1 
        << " with error code " << cancelCheck << endl;
    }
  }

 //print out how many customers did not receive service 
  cout << "# customers who didn't receive a service = " << shop.get_cust_drops()
       << endl;
  return 0;
}

//barber thread execution steps
void *barber(void *arg) {
  ThreadParam &param = *(ThreadParam *)arg;
  Shop &shop = *(param.shop);
  int id = param.id;
  int service_time = param.service_time;
  delete &param;

  while (true) {
    shop.helloCustomer(id); // pick up new customer
    usleep(service_time);  //sleep for specified service time
    shop.byeCustomer(id); // release the customer
  }
  
  return nullptr;
}

//customer thread execution steps
void *customer(void *arg) {
  ThreadParam &param = *(ThreadParam *) arg;
  Shop &shop = *(param.shop);
  int id = param.id;
  delete &param;
  
  // if assigned to barber i then wait for service to finish
  // -1 means did not get barber
  int barber = -1;
  if ((barber = shop.visitShop(id)) != -1) {
    shop.leaveShop(id, barber);
  }

  return nullptr;
}
