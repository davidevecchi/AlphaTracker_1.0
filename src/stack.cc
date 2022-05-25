#include <iostream>
#include <fstream>
#include "console.h"
#include "stack.h"

using namespace std;

static int last_id = 1;

void init(stack &s) {
    s = nullptr;
}

void deinit(stack &s) {
    while (!empty(s))
        pop(s);
}

bool empty(const stack &s) {
    return s == nullptr;
}

void push(stack &s, const Ray ray) {
    node *n = new node;
    n->id = last_id++;
    n->count = 0;
    n->ray = ray;
    n->alive = true;
    n->life = 1;
    n->next = s;
    s = n;
}

bool edit(stack &s, int id, int counter) {
    bool ret = false;
    if (!empty(s)) {
        if (s->id == id) {
            s->count = counter;
            ret = true;
        } else {
            ret = edit(s->next, id, counter);
        }
    }
    return ret;
}

bool edit(stack &s, int id, bool alive) {
    bool ret = false;
    if (!empty(s)) {
        if (s->id == id) {
            if (alive)
                (s->life) += 1;
            else
                s->alive = false;
            ret = true;
        } else
            ret = edit(s->next, id, alive);
    }
    return ret;
}

bool edit(stack &s, int id, const Ray ray) {
    bool ret = false;
    if (!empty(s)) {
        if (s->id == id) {
            s->ray = ray;
            ret = true;
        } else
            ret = edit(s->next, id, ray);
    }
    return ret;
}

bool pop(stack &s) {
    bool ret = false;
    if (!empty(s)) {
        node *tmp = s;
        s = s->next;
        delete tmp;
        ret = true;
    }
    return ret;
}

static bool pop_middle(stack &s, int id) {
    bool ret = false;
    if (!empty(s) && !empty(s->next)) {
        if ((s->next)->id == id) {
            node *tmp = s->next;
            s->next = (s->next)->next;
            delete tmp;
            ret = true;
        } else
            ret = pop_middle(s->next, id);
    }
    return ret;
}

bool pop(stack &s, int id) {
    bool ret = false;
    if (!empty(s)) {
        if (s->id == id)
            ret = pop(s);
        else if (!empty(s->next))
            ret = pop_middle(s, id);
    }
    return ret;
}

bool top_ray(const stack &s, Ray &ray) {
    bool ret = false;
    if (!empty(s)) {
        ray = s->ray;
        ret = true;
    }
    return ret;
}

bool top_id(const stack &s, int &id) {
    bool ret = false;
    if (!empty(s)) {
        id = s->id;
        ret = true;
    }
    return ret;
}

bool top_count(const stack &s, int &count) {
    bool ret = false;
    if (!empty(s)) {
        count = s->count;
        if (count == 0)
            ret = top_count(s->next, count);
        else
            ret = true;
    }
    return ret;
}


bool count_act(const stack &s, int &count) {
    bool ret = false;
    if (!empty(s)) {
        if (s->count == 0)
            ret = count_act(s->next, ++count);
        else {
            count += s->count;
            ret = true;
        }
    }
    return ret;
}

bool dig_life(const stack &s, int id, int &life) {
    bool ret = false;
    if (!empty(s) && id <= s->id) {
        if (s->id == id) {
            life = s->life;
            ret = true;
        } else
            ret = dig_life(s->next, id, life);
    }
    return ret;
}

bool dig_count(const stack &s, int id, int &count) {
    bool ret = false;
    if (!empty(s) && id <= s->id) {
        if (s->id == id) {
            count = s->count;
            ret = true;
        } else
            ret = dig_count(s->next, id, count);
    }
    return ret;
}

bool dig_ray(const stack &s, int id, Ray &ray) {
    bool ret = false;
    if (!empty(s) && id <= s->id) {
        if (s->id == id) {
            ray = s->ray;
            ret = true;
        } else
            ret = dig_ray(s->next, id, ray);
    }
    ray.calcM();
    return ret;
}

void print(const stack &s) {
    if (!empty(s)) {
        print(s->next);
        if (s->alive == true)
            cout << c_bold << f_grn;
        cout << s->count << ".\t"
             << s->ray.frm << "\t"
             << s->ray.len << "\t"
             << s->ray.dir << "\t"
             << s->life << "\n";
//	         << s ->id << "\t"
//	         << "(" << s->ray.Ax << ',' << s->ray.Ay << ") \t"
//	         << "(" << s->ray.Bx << ',' << s->ray.By << ") \t"
//	         << s -> alive << endl;
        cout << c_normal;
    }
}

static void print_ric(const stack &s, int x, int y, int dim, int count, int i) {
    if (!empty(s) && i < dim) {
        gotoxy(x, y + dim - 1 - i);
        if (i < dim - count) {
            cout << tab(24);
            print_ric(s, x, y, dim, count, i + 1);
        } else {
            if (s->alive == true) {
                if (s->count > 0)
                    cout << f_grn;
                else
                    cout << f_ylw;
            }
            cout << c_bold
                 << s->count << tab(4, intlen(s->count)) << c_dl_bold
                 << s->ray.frm << tab(10, intlen(s->ray.frm) + intlen(s->ray.len))
                 << s->ray.len << tab(5, intlen(s->ray.dir))
                 << s->ray.dir << tab(5, intlen(s->life))
                 << s->life << c_normal;
            print_ric(s->next, x, y, dim, count, i + 1);
        }
    }
}

void print(const stack &s, int x, int y, int dim) {
    int count = 0;
    count_act(s, count);
    print_ric(s, x, y, dim, count, 0);
}

//void print (const stack &s, int id)
//{
//	if (!empty(s))
//	{
//	    cout << s->count << ".\t"
//	         << s->id << "\t"
//	         << "(" << s->ray.Ax << ',' << s->ray.Ay << ") \t"
//	         << "(" << s->ray.Bx << ',' << s->ray.By << ") \t"
//	         << s->ray.len << "\t"
//	         << s->ray.dir << "\t"
//	         << s->ray.frm << "\t"
//	         << s->life << "\t"
//	         << s->alive << endl;
//	    print (s -> next, id);
//	}
//}

static fstream myout;

static void save_ric(const stack &s) {
    if (!empty(s)) {
        save_ric(s->next);
        myout << s->count << "\t"
              << s->ray.A.x << "\t" << s->ray.A.y << "\t"
              << s->ray.B.x << "\t" << s->ray.B.y << "\t"
              << s->ray.len << "\t"
              << s->ray.dir << "\t"
              << s->ray.frm << "\t"
              << s->life << "\n";
    }
}

void save(const stack &s, const string& path) {
    myout.open(path + "data.txt", ios::out);
    myout << "ID\tAx\tAy\tBx\tBy\tlen\tang\tfrm\tlife\n";
    if (!empty(s))
        save_ric(s);
    myout.close();
}


