#ifndef STACK_H
#define STACK_H

#include <opencv2/opencv.hpp>
#include "functions.h"

struct Ray {
    cv::Point A, B;
    int len, dir, frm;
    cv::Point M;
    void calcM() {
        M = med(A, B);
    }
} __attribute__((aligned(64)));

struct node {
    int id;
    int count;
    Ray ray;
    bool alive;
    int life;
    node *next;
} __attribute__((aligned(64)));

typedef node *stack;

void init(stack &s);

void deinit(stack &s);

void push(stack &s, Ray ray);

bool edit(stack &s, int id, int counter);

bool edit(stack &s, int id, bool alive);

bool edit(stack &s, int id, Ray ray);

bool pop(stack &s);

bool pop(stack &s, int id);

bool empty(const stack &s);

bool top_count(const stack &s, int &count);

bool top_id(const stack &s, int &id);

bool top_ray(const stack &s, Ray &ray);

bool dig_life(const stack &s, int id, int &life);

bool dig_count(const stack &s, int id, int &count);

bool dig_ray(const stack &s, int id, Ray &ray);

bool count_act(const stack &s, int &count);

void print(const stack &s);

void print(const stack &s, int x, int y, int dim);

/*void print     (const stack &s, int id);*/
void save(const stack &s, const string &path);


#endif // STACK_H
