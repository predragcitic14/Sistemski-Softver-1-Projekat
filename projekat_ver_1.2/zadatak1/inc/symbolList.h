#include <iostream>
#include <cstdlib>
using namespace std;

struct symbol
{
  string label;
  string section;
  int offset;
  int local;
  int ordNumber;
  int size;
  int defined;
  symbol(string l, string s, int o, int loc, int ord, int def)
  {
    label = l;
    section = s;
    offset = o;
    local = loc;
    ordNumber = ord;
    size = 0;
    defined = def;
  }
};
void printSymbolTable(list<symbol> l, std::ofstream &out)
{
  list<symbol>::iterator it;

  out << setw(20) << left << "Label" << setw(20) << left
      << "Section" << setw(20)
      << "Offset(decimal)" << setw(20)
      << "Type And Binding" << setw(20)
      // << "Defined" << setw(20)
      << "Ordinal Number" << setw(20)
      << "Section size" << endl;
  out << "------------------------------------------------------------------------------------------------------------------------------------" << endl;
  for (it = l.begin(); it != l.end(); it++)
  {
    out << setw(20) << left << it->label << setw(20) << left << it->section << setw(20) << left << it->offset;
    (it->local == 1) ? (out << setw(20) << left << "Local") : (out << setw(20) << left << "Global");

    // (it->defined == 1) ? (out << setw(20) << left << "YES") : (out << setw(20) << left << "NO");
    out << setw(20) << left << it->ordNumber;
    // cout<< "Section: " << it->section << endl;
    if (it->size > 0)
      out << setw(20) << left << it->size << endl;
    else
      out << setw(20) << left << "X" << endl;
  }
}
