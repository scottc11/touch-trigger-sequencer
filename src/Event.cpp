#include<stdio.h>
#include<stdlib.h>


// linked lists of structs with pointers to the next struct in the list

typedef struct Event {
  int channel;
  long duration;       // the duration of each event in microseconds
  long position;       // the position of each event relative to the current position in the loop (microseconds)
  struct Event *next;  // pointer to the 'next' Event to occur (linked list)
} Event;

// function which returns a pointer to an Event
struct Event* newEvent(int chan, long pos, long dur) {
  struct Event *e = malloc(sizeof(struct Event));
  e->channel = chan;
  e->position = pos;
  e->duration = dur;
  e->next = NULL;
  return e;
}



int main() {
  struct Event *e1 = newEvent(1, 75, 25);
  struct Event *e2 = newEvent(1, 200, 25);
  struct Event *e3 = newEvent(1, 800, 25);

  e1->next = e2;
  e2->next = e3;

  struct Event *first = e1;
  struct Event *nextEvent = first;

  while (nextEvent != NULL) {
    printf("Event: %d\n", nextEvent->channel);
    nextEvent = nextEvent->next;
  }

  struct Event *prev = first;
  while (nextEvent != NULL) {
    prev = nextEvent;
    nextEvent = nextEvent->next;
    free(prev);
  }
}

void loop() {
  // if new Event, iterate over all Events and and assign the pointers

}
