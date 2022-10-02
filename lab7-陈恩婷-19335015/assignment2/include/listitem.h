#ifndef LISTITEM
#define LISTITEM

class ListItem
{
public:
    ListItem *previous;
    char *start;
    char *end;
    ListItem *next;
};

#endif
