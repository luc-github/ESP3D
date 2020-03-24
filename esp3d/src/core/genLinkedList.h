/*
  genlinkedlist.h - ESP3D simple generic linked list class
  * inspired by great https://github.com/ivanseidel/LinkedList

  Copyright (c) 2018 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef _GENLINKEDLIST_H
#define _GENLINKEDLIST_H

template<class T> struct GenNode {
    T data;
    GenNode<T> *next;
    GenNode<T> *prev;
};

template <typename T> class GenLinkedList
{
public:
    GenLinkedList();
    ~GenLinkedList();
    //Clear list
    void clear();
    //number of GenNodes
    size_t count();
    //add data at the end of list
    bool push(T data);
    //get data at end of list and remove from list
    T pop();
    //add data at the begining of list
    bool shift(T data);
    //get data from begining of list and remove from list
    T unshift();
    //get data from index position
    T get(size_t index);
    //get head data
    T getFirst();
    //get tail data
    T getLast();

private:
    //number of GenNode
    size_t _nb;
    //First GenNode
    GenNode<T> *_head;
    //Last GenNode
    GenNode<T>  *_tail;
};

//Constructor
template <typename T>GenLinkedList<T>::GenLinkedList()
{
    _nb = 0;
    _head = nullptr;
    _tail = nullptr;
}

//Destructor
template <typename T>GenLinkedList<T>::~GenLinkedList()
{
    clear();
}

//clear list of all GenNodes
template<typename T>void GenLinkedList<T>::clear()
{
    GenNode<T> *current;
    //delete from first to last
    while (_head != nullptr) {
        current = _head;
        _head = _head->next;
        current->data = T();
        delete current;
    }
    _tail = _head;
    _nb = 0;
}

//Number of GenNodes
template<typename T>size_t GenLinkedList<T>::count()
{
    return _nb;
}

//add to end of list
template<typename T> bool GenLinkedList<T>::push (T data)
{
    GenNode<T> *ptr = new GenNode<T>();
    if (!ptr) {
        return false;
    }
    ptr->data = data;
    ptr->next = nullptr;
    ptr->prev = nullptr;
    _nb++;
    //Check if already have element in list
    if (_head) {
        _tail->next = ptr;
        ptr->prev = _tail;
        _tail = ptr;
    } else { // no element _head = _tail
        _head = ptr;
        _tail = _head;
    }
    return true;
}
//take out from end of list
template<typename T>T GenLinkedList<T>::pop()
{
    if (_head == nullptr) {
        return T();
    }
    T data = _tail->data;

    _nb--;
    if (_head == _tail) {
        _head->data = T();
        delete (_head);
        _head = nullptr;
        _tail = _head;
    } else {
        GenNode<T> *ptr = _tail;
        _tail = _tail->prev;
        _tail->next = nullptr;
        ptr->data = T();
        delete (ptr);
    }
    return data;
}

//add to head of list
template<typename T> bool GenLinkedList<T>::shift (T data)
{

    GenNode<T> *ptr = new GenNode<T>();
    if (!ptr) {
        return false;
    }
    ptr->data = data;
    ptr->next = nullptr;
    ptr->prev = nullptr;
    _nb++;
    //Check if already have element in list
    if (_head) {
        ptr->next = _head;
        _head->prev = ptr;
        _head = ptr;
    } else { // no element _head = _tail
        _head = ptr;
        _tail = _head;
    }
    return true;
}

//take out from Head
template<typename T>T GenLinkedList<T>::unshift()
{
    if (_head == nullptr) {
        return T();
    }
    T data = _head->data;
    _nb--;
    if (_head == _tail) {
        _head->data = T();
        delete (_head);
        _head = nullptr;
        _tail = _head;
    } else {
        GenNode<T> *ptr = _head;
        _head = _head->next;
        _head->prev = nullptr;
        ptr->data = T();
        delete (ptr);
    }
    return data;
}

//get data from position (do not remove from list)
template<typename T>T GenLinkedList<T>::get(size_t index)
{
    if ((_head == nullptr) || (index > _nb)) {
        return T();
    }
    GenNode<T> *ptr = _head;
    for (size_t pos = 0; pos < index; pos++) {
        ptr = ptr->next;
    }
    return ptr->data;
}


//get head data (do not remove from list)
template<typename T>T GenLinkedList<T>::getFirst()
{
    if (_head == nullptr) {
        return T();
    }
    return _head->data;
}

//get tail data (do not remove from list)
template<typename T>T GenLinkedList<T>::getLast()
{
    if (_head == nullptr) {
        return T();
    }
    return _tail->data;
}

#endif //_GENLINKEDLIST_H
