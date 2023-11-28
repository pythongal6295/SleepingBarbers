//-----------------------shop.h----------------------------
//Kelly Kauffman
//CSS 503
//Date created 5.1.23
//Date last edited 5.7.23

//Description: header file for the Shop class
//Includes methods for the barber and customer actions, in addition
//to helper methods to print output to the screen.

//------------------------------------------------------------

#ifndef SHOP_H_
#define SHOP_H_
#include <iostream>
#include <pthread.h>
#include <queue>
#include <sstream>
#include <string>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultBarbers 1

class Shop {
public:
  //parameterized constructor
  Shop(int num_barbers, int num_chairs)
      : max_waiting_cust_((num_chairs >= 0) ? num_chairs : kDefaultNumChairs),
        barber_count_((num_barbers > 0) ? num_barbers : kDefaultBarbers),
        customer_in_chair_(0),
        cust_drops_(0) {

    //dynamically allocate arrays
    in_service_ = new bool [barber_count_];
    money_paid_ = new bool [barber_count_];
    customer_in_chair_ = new int[barber_count_];

    cond_customer_served_ = new pthread_cond_t[barber_count_];
    cond_barber_paid_ = new pthread_cond_t[barber_count_];
    cond_barber_sleeping_ = new pthread_cond_t[barber_count_];
        
    init(barber_count_);
  };

  //default constructor
  Shop()
      : max_waiting_cust_(kDefaultNumChairs), customer_in_chair_(0),
        barber_count_(kDefaultBarbers),
        cust_drops_(0) {

    //dynamically allocate arrays
    in_service_ = new bool [barber_count_];
    money_paid_ = new bool [barber_count_];
    customer_in_chair_ = new int[barber_count_];

    cond_customer_served_ = new pthread_cond_t[barber_count_];
    cond_barber_paid_ = new pthread_cond_t[barber_count_];
    cond_barber_sleeping_ = new pthread_cond_t[barber_count_];
        
    init(barber_count_);
  };

  int visitShop(int id); // return the barber id or -1 if no barber found, customer func
  void leaveShop(int id, int barber_id); //customer func
  void helloCustomer(int id);  //barber func
  void byeCustomer(int id);  //barber func
  int get_cust_drops() const;  //return customers that didn't get service

private:
  const int max_waiting_cust_; // the max number of threads that can wait
  int *customer_in_chair_;  //pointer to array of barber chairs
  int barber_count_;  //total barbers
  bool *in_service_;  //pointer to array for in_service_ variables
  bool *money_paid_;  //pointer to array for money_paid_ variables
  queue<int> waiting_chairs_; // includes the ids of all waiting threads
  int cust_drops_;  //number of customers that didn't get service

  // Mutexes and condition variables to coordinate threads
  // mutex_ is used in conjuction with all conditional variables
  pthread_mutex_t mutex_;
  pthread_cond_t cond_customers_waiting_;
  pthread_cond_t *cond_customer_served_;  //array of condition variables
  pthread_cond_t *cond_barber_paid_;  //array of condition variables
  pthread_cond_t *cond_barber_sleeping_;  //array of condition variables

  void init(int);
  string int2string(int i);  //change int to string
  void print(int person, bool type, string message);  //print barber or customer step
};
#endif
