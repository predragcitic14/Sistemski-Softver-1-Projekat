#ifndef RELOCATIONLIST_H_
#define RELOCATIONLIST_H_

#include <iostream>
#include <cstdlib>
using namespace std;

struct relocation
{
    string section;
    int offset;
    string relType;
    int ordNumber;
    int addend;
    relocation(string sec, int o, string rt, int on, int add)
    {
        section = sec;
        offset = o;
        relType = rt;
        ordNumber = on;
        addend = add;
    }
};

void printRelocationTable(list<relocation> l, std::ofstream &out)
{

    out << setw(20) << left << "Section" << setw(20)
        << "Offset(decimal)" << setw(20)
        << "Relocation Type" << setw(20)
        << "Ordinal Number" << setw(20)
        /* << "Addend" */
        << endl;
    out << "---------------------------------------------------------------------------" << endl;
    list<relocation>::iterator it;
    for (it = l.begin(); it != l.end(); it++)
    {
        out << setw(20) << left << it->section << setw(20) << left << it->offset << setw(20) << left << it->relType;
        out << setw(20) << left << it->ordNumber /*<< setw(20) << left << it->addend */ << endl;
    }

    // out.close();

    // cout << setw(20) << "Section" << setw(20)
    //      << "Offset(decimal)" << setw(20)
    //      << "Relocation Type" << setw(20)
    //      << "Ordinal Number" << setw(20)
    //      /* << "Addend" */
    //      << endl;
    // cout << "---------------------------------------------------------------------------" << endl;
    // // list<relocation>::iterator it;
    // for (it = l.begin(); it != l.end(); it++)
    // {
    //     cout << setw(20) << left << it->section << setw(20) << left << it->offset << setw(20) << left << it->relType;
    //     cout << setw(20) << left << it->ordNumber /*<< setw(20) << left << it->addend */ << endl;
    // }
}

#endif