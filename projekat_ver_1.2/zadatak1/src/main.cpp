#include <fstream>
#include <iomanip>
#include <bits/stdc++.h>
#include "../inc/initLists.h"
#include "../inc/symbolList.h"
#include "../inc/regExpressions.h"
#include "../inc/relocationList.h"

/*
RELOKACIONI ZAPISI(sekcija, ofset, tip relokacije, simbol, addend)
RODATA(ofset, sadrzaj , objasnjenje)
TEXT(ofset (decimalno), sadrzaj (hexa), objasnjenje,)
*/

using namespace std;

int LOCATIONCOUNTER = 0;    // reseted when new saction introduced
string CURRENTSECTION = ""; // string to be given to new symbol when introduced
int ORDNUM = 0;             // orinal number of each symbol
int GLC = 0;                // global location counter; this counter will not be reseted when new section is introduced
list<symbol> symbol_table;  // symbol table; formed in first pass
list<relocation> relocation_table;
smatch match;                // this variable holds all matched regexes
string line;                 // current line that assembler is processing
unsigned int lineNumber = 1; // ordinal number of line from given .s file; used primarily for error output
list<symbol>::iterator it;   // iterator class for easy navigating through symbol table
bool found = false;          // bool that will give information if symbol already exists in symbol table
int status;                  // error handling
short skipNum = 0;           // this variable will be used when .skip directive is processed
bool comment;                // bool that gives us information that comment is in place and we should start processing the line bellow
bool first_Pass;
bool lineCorrect;

vector<string> sections;
vector<string> sectionContent;
string absoluteSection;

int checkGlobal(int i, vector<string> tokens, int defined)
{

  found = false;

  string globals = "";
  for (int j = i + 1; j < tokens.size(); j++)
  {
    if (tokens[j].substr(0, 1) == "#")
      break; // we reached comments
    globals.append(tokens[j]);
  }

  vector<string> symbolTokens;
  stringstream check2(globals);
  string intermediate1;
  while (getline(check2, intermediate1, ','))
  {
    symbolTokens.push_back(intermediate1);
  }

  for (int k = 0; k < symbolTokens.size(); k++)
  {
    if (!regex_match(symbolTokens[k], match, varReg))
    {
      cout << "GRESKA! POGRESAN FORMAT PROMENLJIVE!" << endl;
      return 404;
    }
    if (regex_match(symbolTokens[k], match, directiveRegex))
    {
      cout << "GRESKA u GLOBAL! JEDNA LINIJA IZVORNOG KODA SADRZI NAJVISE JEDNU ASEMBLERSKU NAREDBU ILI DIREKTIVU!" << endl;
      return 404;
    }

    for (it = symbol_table.begin(); it != symbol_table.end(); it++)
    {
      if (it->label == symbolTokens[k])
      {
        if (it->local == 1 && it->defined == 0)
        {
          found = true;
          it->local = 0;
          it->defined = 1;
        }
        else
        {
          cout << "GRESKA! Ne smete dva puta definisati isti simbol!" << endl;
          return 404;
        }
      }
    }
    if (!found)
    {
      symbol_table.push_back(symbol(symbolTokens[k], "UND", 0, 0, ++ORDNUM, defined));
    }
    // else
    // {
    //   cout << "GRESKA! DVA PUTA STE DEFINISALI ISTI SIMBOL!" << endl;
    //   return 404;
    // }
  }
  return 0;
};
int checkSection(int i, vector<string> tokens)
{
  if (regex_match(tokens[i], match, directiveRegex))
  {
    cout << "GRESKA u SECTION! JEDNA LINIJA IZVORNOG KODA SADRZI NAJVISE JEDNU ASEMBLERSKU NAREDBU ILI DIREKTIVU!" << endl;
    return 404;
  }

  list<symbol>::iterator it;

  for (it = symbol_table.begin(); it != symbol_table.end(); it++)
  {
    if (it->label == CURRENTSECTION)
    {
      it->size = LOCATIONCOUNTER;
      break;
    }
  }
  CURRENTSECTION = tokens[i];
  LOCATIONCOUNTER = 0;
  symbol_table.push_back(symbol(tokens[i], CURRENTSECTION, 0, 1, ++ORDNUM, 1));
  sections.push_back(tokens[i]);
  return 0;
}
int checkWord(int i, vector<string> tokens)
{
  string words = "";
  for (int j = i + 1; j < tokens.size(); j++)
    // promenjeno to da sam izbacio proveru da li je "" ili "#"
    words.append(tokens[j]);
  vector<string> tokens1;
  stringstream check2(words);
  string intermediate1;

  while (getline(check2, intermediate1, ','))
  {
    tokens1.push_back(intermediate1);
  }

  if (first_Pass)
  {
    if (CURRENTSECTION == "" || CURRENTSECTION == "UND")
    {
      cout << "GRESKA u WORD! Sadrzaj mora biti u nekoj od sekcija" << endl;
      return 404;
    }
    for (int k = 0; k < tokens1.size(); k++)
    {
      if (!regex_match(tokens1[k], match, varReg) && !regex_match(tokens1[k], match, numberRegex))
      {
        cout << "GRESKA! POGRESAN FORMAT PROMENLJIVE!" << endl;
        return 404;
      }
      if (regex_match(tokens1[k], match, directiveRegex))
      {
        cout << "GRESKA u WORD! JEDNA LINIJA IZVORNOG KODA SADRZI NAJVISE JEDNU ASEMBLERSKU NAREDBU ILI DIREKTIVU!" << endl;
        return 404;
      }

      list<symbol>::iterator it;
      bool found = false;
      for (it = symbol_table.begin(); it != symbol_table.end(); it++)
      {
        if (it->label == tokens1[k])
        {
          // it->local = 0;
          found = true;
          break;
          // cout << "GRESKA! Ne smete dva puta definisati isti simbol!" << endl;
          // return 404;
        }
      }
      if (!found && regex_match(tokens1[k], match, varReg))
      {
        symbol_table.push_back(symbol(tokens1[k], CURRENTSECTION, LOCATIONCOUNTER, 1, ++ORDNUM, 0));
        LOCATIONCOUNTER += 2;
        GLC += 2;
      }
      else if (!found && regex_match(tokens1[k], match, numberRegex))
      {
        LOCATIONCOUNTER += 2;
        GLC += 2;
      }
      else
      {
        LOCATIONCOUNTER += 2;
        GLC += 2;
      }
    }
  }
  else // we are in the second pass
  {
    for (int k = 0; k < tokens1.size(); k++)
    {
      list<symbol>::iterator it;
      bool found = false;
      for (it = symbol_table.begin(); it != symbol_table.end(); it++)
      {
        if (it->label == tokens1[k])
        {
          found = true;
          if (it->defined == 1)
          {
            std::stringstream sstream;
            sstream << std::hex << setfill('0') << setw(4) << it->offset;
            string result = sstream.str();
            sectionContent[sectionContent.size() - 1].append(result.substr(2, 2));
            sectionContent[sectionContent.size() - 1].append(result.substr(0, 2));
            sectionContent[sectionContent.size() - 1].append(" ");

            int ordnum = it->ordNumber;
            // VAZNA NAPOMENA! AKO BUDES DODAVAO UND U TABELU SIMBOLA OVO NECE RADITI
            list<symbol>::iterator it1;
            for (it1 = symbol_table.begin(); it1 != symbol_table.end(); it1++)
            {
              if (it1->label == it->section)
              {
                ordnum = it1->ordNumber;
                break;
              }
            }

            relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER, "R_386_16", ordnum, 0));
          }
          else
          {
            cout << " GRESKA SIMBOL MORA BITI DEFINISAN " << lineNumber << endl;
            exit(-1);
            // sectionContent[sectionContent.size() - 1].append("0000 ");
            // relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER, "R_X86_64", it->ordNumber, 0));
          }
          LOCATIONCOUNTER += 2;
          GLC += 2;
        }
      }
      if (!found && regex_match(tokens1[k], match, numberRegex))
      {
        short value;
        if (tokens1[k].substr(0, 1) != "0")
        {
          value = (short)stoi(tokens1[k]);
        }
        else if (tokens1[k].substr(0, 2) == "0x")
        {
          value = (short)stoi(tokens1[k], 0, 16);
        }
        else if (tokens1[k].substr(0, 1) == "0")
        {
          value = (short)stoi(tokens1[k], 0, 8);
        }

        std::stringstream sstream;
        sstream << std::hex << setfill('0') << setw(4) << value;
        string result = sstream.str();
        sectionContent[sectionContent.size() - 1].append(result.substr(2, 2));
        sectionContent[sectionContent.size() - 1].append(result.substr(0, 2));
        sectionContent[sectionContent.size() - 1].append(" ");
        LOCATIONCOUNTER += 2;
        GLC += 2;
      }
    }
  }
  return 0;
}
int checkEqu(string symbolAndLiteral)
{
  vector<string> tokens1;
  stringstream check2(symbolAndLiteral);
  string intermediate1;

  // Tokenizing w.r.t. space ' '
  while (getline(check2, intermediate1, ','))
  {
    tokens1.push_back(intermediate1);
  }

  list<symbol>::iterator it;
  bool found = false;
  for (it = symbol_table.begin(); it != symbol_table.end(); it++)
  {
    if (it->label == tokens1[0])
    {
      it->local = 0;
      it->defined = 1;
      found = true;
      cout << "GRESKA na liniji " << lineNumber << " ! Ne smete dva puta definisati isti simbol!" << endl;
      return 404;
    }
  }
  if (!found)
  {
    short value;
    if (tokens1[1].substr(0, 1) != "0")
    {
      value = (short)stoi(tokens1[1]);
    }
    else if (tokens1[1].substr(0, 2) == "0x")
    {
      value = (short)stoi(tokens1[1], 0, 16);
    }
    else if (tokens1[1].substr(0, 1) == "0")
    {
      value = (short)stoi(tokens1[1], 0, 8);
    }
    std::stringstream sstream;
    sstream << std::hex << setfill('0') << setw(4) << value;
    string result = sstream.str();
    absoluteSection.append(result.substr(2, 2));
    absoluteSection.append(result.substr(0, 2));
    absoluteSection.append(" ");
    symbol_table.push_back(symbol(tokens1[0], "ABS", value, 1, ++ORDNUM, 1));
  }
  return 0;
}

void firstPass(ifstream &f, std::ofstream &out)
{
  // KASNIJE DODAJ OVO ZA OBRADU SINTAKSNIH GRESAKA
  // cout << "Pocinje prvi prolaz" << endl;
  // cout << "-------------------------------" << endl;
  while (getline(f, line))
  {
    // cout << "-------------------------------" << endl;
    // if (line.substr(0, 1) != " " && line.substr(0, 1) != "\t")
    //   cout << "\t" << line << endl;
    // else
    //   cout << line << endl;
    // cout << "-------------------------------" << endl;
    vector<string> tokens;
    skipNum = 0;
    stringstream check1(line);
    string intermediate;
    comment = false;
    lineCorrect = false;
    // Tokenizing of current line using space
    while (getline(check1, intermediate, ' '))
    {
      intermediate.erase(std::remove_if(intermediate.begin(), intermediate.end(), ::isspace), intermediate.end());
      if (intermediate == "#" || intermediate.substr(0, 1) == "#")
        comment = true;
      if (intermediate != "" && intermediate != "\t" && intermediate != " " && !comment)
      {
        tokens.push_back(intermediate);
      }
    }
    if (tokens.size() == 0) /*ignore this line */
      cout << "";
    // ---------------------------------------------------------------------------------------------------------------
    // LABEL PROCESSING
    else
    {
      if (regex_match(tokens[0], match, labelRegex))
      {
        found = false;
        lineCorrect = true;
        for (it = symbol_table.begin(); it != symbol_table.end(); it++)
        {
          string appendedVar = it->label;
          appendedVar.append(":");
          if (appendedVar == tokens[0] /*&& it->section=="UND"*/)
          {
            if (it->defined == 1)
            {
              cout << " NE SMETE DVA PUTA DEFINISATI ISTI SIMBOL!" << endl;
              exit(-1);
            }
            else
            {
              it->offset = LOCATIONCOUNTER;
              it->section = CURRENTSECTION;
              it->defined = 1;
              found = true;
            }
          }
        }
        if (!found)
        {
          string lab = tokens[0];
          lab.erase(lab.size() - 1, 1);
          symbol_table.push_back(symbol(lab, CURRENTSECTION, LOCATIONCOUNTER, 1, ++ORDNUM, 1));
        }
      }
      //----------------------------------------------------------------------------------------------------------------
      //.global DIRECTIVE PROCESSING

      if (regex_match(tokens[0], match, globalRegex) && tokens.size() >= 2)
      {
        lineCorrect = true;
        int defined = 0;
        if (tokens[0] == ".extern")
          defined = 1;

        // cout << "DEFINISI NA LINIJI " << lineNumber << endl;
        status = checkGlobal(0, tokens, defined);
        if (status == 404)
          exit(-1);
      }

      if (regex_match(tokens[0], match, labelRegex) && tokens.size() >= 3 && regex_match(tokens[1], match, globalRegex))
      {
        int defined = 0;
        if (tokens[0] == ".extern")
          defined = 1;
        status = checkGlobal(1, tokens, defined);
        if (status == 404)
          exit(-1);
      }
      //----------------------------------------------------------------------------------------------------------------

      // .section DIRECTIVE PROCESSING
      if (regex_match(tokens[0], match, sectionRegex) && tokens.size() == 2)
      {
        lineCorrect = true;
        status = checkSection(1, tokens);
        if (status == 404)
          exit(-1);
      }

      if (tokens.size() == 3 && tokens[1] == ".section" && regex_match(tokens[0], match, labelRegex))
      {
        status = checkSection(2, tokens);
        if (status == 404)
          exit(-1);
      }
      //----------------------------------------------------------------------------------------------------------------

      // .word DIRECTIVE PROCESSING
      if (regex_match(tokens[0], match, wordRegex) && tokens.size() >= 2)
      {
        lineCorrect = true;
        status = checkWord(0, tokens);
        if (status == 404)
          exit(-1);
      }
      if (regex_match(tokens[0], match, labelRegex) && tokens.size() >= 3 && tokens[1] == ".word")
      {
        status = checkWord(1, tokens);
        if (status == 404)
          exit(-1);
      }

      //----------------------------------------------------------------------------------------------------------------

      // .skip DIRECTIVE PROCESSING
      if (tokens[0] == ".skip" && tokens.size() == 2)
      {
        lineCorrect = true;
        if (!regex_match(tokens[1], match, numberRegex))
        {
          cout << " GRESKA U .SKIP DIREKTIVI!" << endl;
          exit(-1);
        }

        if (tokens[1].substr(0, 1) != "0")
        {
          skipNum = (short)stoi(tokens[1]);
        }
        else if (tokens[1].substr(0, 2) == "0x")
        {
          skipNum = (short)stoi(tokens[1], 0, 16);
        }
        else if (tokens[1].substr(0, 1) == "0")
        {
          skipNum = (short)stoi(tokens[1], 0, 8);
        }

        LOCATIONCOUNTER += skipNum;
        GLC += skipNum;
      }
      if (regex_match(tokens[0], match, labelRegex) && tokens.size() == 3 && tokens[1] == ".skip")
      {
        if (!regex_match(tokens[2], match, numberRegex))
        {
          cout << " GRESKA U .SKIP DIREKTIVI!" << endl;
          exit(-1);
        }
        if (tokens[2].substr(0, 1) != "0")
        {
          skipNum = (short)stoi(tokens[2]);
        }
        else if (tokens[2].substr(0, 2) == "0x")
        {
          skipNum = (short)stoi(tokens[2], 0, 16);
        }
        else if (tokens[2].substr(0, 1) == "0")
        {
          skipNum = (short)stoi(tokens[2], 0, 8);
        }
        LOCATIONCOUNTER += skipNum;
        GLC += skipNum;
      }

      //--------------------------------------------------------------------------------------------------------------
      //.equ DIRECTIVE PROCESSING
      if (tokens[0] == ".equ" ||
          (regex_match(tokens[0], match, labelRegex) && tokens.size() >= 3 && tokens[1] == ".equ"))
      {
        lineCorrect = true;
        int i = (regex_match(tokens[0], match, labelRegex)) ? 1 : 0;
        string symbolAndLiteral = "";
        for (int j = i + 1; j < tokens.size(); j++)
        {
          symbolAndLiteral.append(tokens[j]);
        }

        if (!regex_match(symbolAndLiteral, match, equRegex))
        {
          cout << "Sintaksna greska na liniji " << lineNumber << " : equ direktiva mora biti u formatu .equ <novi_simbol>, <literal>" << endl;
          exit(-1);
        }
        int status = checkEqu(symbolAndLiteral);
        if (status == 404)
          exit(-1);
      }

      //--------------------------------------------------------------------------------------------------------------
      //.end DIRECTIVE PROCESSING
      if (tokens[0] == ".end" || (regex_match(tokens[0], match, labelRegex) && tokens.size() >= 2 && tokens[1] == ".end"))
      {

        lineCorrect = true;
        list<symbol>::iterator it;

        for (it = symbol_table.begin(); it != symbol_table.end(); it++)
        {
          if (it->label == CURRENTSECTION)
          {
            it->size = LOCATIONCOUNTER;
            break;
          }
          if (it->defined == 0)
          {
            cout << "Simbol " << it->label << " nije definisan!!!!" << endl;
            exit(-1);
          }
        }
        // cout << "PRVI PROLAZ GOTOV" << endl;
        // cout << "-------------------------------" << endl
        //      << endl
        //      << endl;
        out << "\tTABELA SIMBOLA" << endl;
        out << endl;
        break;
      }
      //----------------------------------------------------------------------------------------------------------------
      // (halt, ret, iret) PROCESSING
      if ((regex_match(tokens[0], match, oneByteCommandsRegex) && tokens.size() == 1) ||
          (tokens.size() == 2 && regex_match(tokens[0], match, labelRegex) && regex_match(tokens[1], match, oneByteCommandsRegex)))
      {
        lineCorrect = true;
        LOCATIONCOUNTER += 1;
        GLC += 1;
      }
      //----------------------------------------------------------------------------------------------------------------
      // (int, xchg, add, sub, mul, div, cmp, not, and, or, xor, test, shl, shr) PROCESSING
      if (regex_match(tokens[0], match, twoByteCommandsRegex) ||
          (tokens.size() >= 3 && regex_match(tokens[0], match, labelRegex) && regex_match(tokens[1], match, twoByteCommandsRegex)))
      {

        lineCorrect = true;
        int i = (regex_match(tokens[0], match, twoByteCommandsRegex)) ? 0 : 1;

        if (tokens[i] == "int" || tokens[i] == "not")
        {
          if (!regex_match(tokens[i + 1], match, registerRegex))
          {
            cout << "Sintaksna greska na liniji " << lineNumber << " : registri r0-r7 smeju da se koriste" << endl;
            exit(-1);
          }
        }
        else
        {
          string regs = "";
          for (int j = i + 1; j < tokens.size(); j++)
          {
            regs.append(tokens[j]);
          }

          if (!regex_match(regs, match, regsRegex))
          {
            cout << "Sintaksna greska na liniji " << lineNumber << " : registri r0-r7 smeju da se koriste, odvojeni zarezom!" << endl;
            exit(-1);
          }
        }
        LOCATIONCOUNTER += 2;
        GLC += 2;
        // cout << "TOKENS SIZE " << tokens.size() << endl;
      }
      //----------------------------------------------------------------------------------------------------------------
      // (push, pop) PROCESSING
      if (regex_match(tokens[0], match, threeByteCommandRegex) ||
          (regex_match(tokens[0], match, labelRegex) && tokens.size() > 2 && regex_match(tokens[1], match, threeByteCommandRegex)))
      {
        lineCorrect = true;
        int i = (regex_match(tokens[0], match, threeByteCommandRegex)) ? 0 : 1;
        if (!regex_match(tokens[i + 1], match, registerRegex))
        {
          cout << "Sintaksna greska na liniji " << lineNumber << " : registri r0-r7 smeju da se koriste" << endl;
          exit(-1);
        }
        LOCATIONCOUNTER += 3;
        GLC += 3;
      }
      //--------------------------------------------------------------------------------------------------------------
      //(call, jmp, jeq, jne, jgt) PROCESSING
      if (regex_match(tokens[0], match, jumpsRegex) ||
          (regex_match(tokens[0], match, labelRegex) && tokens.size() > 2 && regex_match(tokens[1], match, jumpsRegex)))
      {
        lineCorrect = true;
        int i = (regex_match(tokens[0], match, jumpsRegex)) ? 0 : 1;
        string operand = "";
        for (int j = i + 1; j < tokens.size(); j++)
        {
          operand.append(tokens[j]);
        }

        // cout << "OPERAND :::::::::::::::" << operand << endl;

        if (regex_match(operand, match, jumps3Regex))
        {
          LOCATIONCOUNTER += 3;
          GLC += 3;
        }
        else if (regex_match(operand, match, jumpsLiteral) || regex_match(operand, match, jumpsSymbol) || regex_match(operand, match, jumpsRegSymbol) || regex_match(operand, match, jumpsRegNumber))
        {
          if (regex_match(operand, match, jumpsSymbol))
          {
            string symb = operand;
            if (symb.substr(0, 1) == "%" || symb.substr(0, 1) == "*")
              symb.erase(0, 1);
            // cout << " STA JA TO URADIH SAD " << symb << endl;
            found = false;
            for (it = symbol_table.begin(); it != symbol_table.end(); it++)
            {
              string currentSymbol = it->label;
              if (currentSymbol == symb /*&& it->section=="UND"*/)
              {
                found = true;
                break;
              }
            }
            if (!found)
            {
              symbol_table.push_back(symbol(symb, "UND", 0, 1, ++ORDNUM, 0));
            }
          }

          if (regex_match(operand, match, jumpsRegSymbol))
          {
            // cout << "MORAMO ODRADITI ONU PROVERU ZA Symbol DEO!!" << endl;
            // cout << match[3].str() << endl;
            string symb = match[3].str();
            found = false;
            for (it = symbol_table.begin(); it != symbol_table.end(); it++)
            {
              string currentSymbol = it->label;
              if (currentSymbol == symb /*&& it->section=="UND"*/)
              {
                found = true;
                break;
              }
            }
            if (!found)
            {
              symbol_table.push_back(symbol(symb, "UND", 0, 1, ++ORDNUM, 0));
            }
          }
          LOCATIONCOUNTER += 5;
          GLC += 5;
        }

        else
        {
          cout << "Sintaksna greska na liniji " << lineNumber << "Operand nije dobro definisan" << endl;
          exit(-1);
        }
      }
      //--------------------------------------------------------------------------------------------------------------
      // (ldr, str) PROCESSING
      if (regex_match(tokens[0], match, memRegex) ||
          (regex_match(tokens[0], match, labelRegex) && tokens.size() > 2 && regex_match(tokens[1], match, memRegex)))
      {
        lineCorrect = true;
        int i = (regex_match(tokens[0], match, memRegex)) ? 0 : 1;
        string operand = "";
        for (int j = i + 1; j < tokens.size(); j++)
        {
          operand.append(tokens[j]);
        }

        if (regex_match(operand, match, mem3Regex))
        {
          LOCATIONCOUNTER += 3;
          GLC += 3;
        }

        else if (regex_match(operand, match, mem5Literal) || regex_match(operand, match, memSymbol) || regex_match(operand, match, memRegSymbol) || regex_match(operand, match, memRegNumber))
        {
          if (regex_match(operand, match, memSymbol))
          {
            string symb = match[2].str();
            if (symb.substr(0, 1) == "%" || symb.substr(0, 1) == "$")
              symb.erase(0, 1);

            found = false;
            for (it = symbol_table.begin(); it != symbol_table.end(); it++)
            {
              string currentSymbol = it->label;
              if (currentSymbol == symb /*&& it->section=="UND"*/)
              {
                found = true;
                break;
              }
            }
            if (!found)
            {
              symbol_table.push_back(symbol(symb, "UND", 0, 1, ++ORDNUM, 0));
            }
          }

          if (regex_match(operand, match, memRegSymbol))
          {
            string symb = match[4].str();

            found = false;
            for (it = symbol_table.begin(); it != symbol_table.end(); it++)
            {
              string currentSymbol = it->label;
              if (currentSymbol == symb /*&& it->section=="UND"*/)
              {
                found = true;
                break;
              }
            }
            if (!found)
            {
              symbol_table.push_back(symbol(symb, "UND", 0, 1, ++ORDNUM, 0));
            }
          }

          LOCATIONCOUNTER += 5;
          GLC += 5;
        }

        else
        {
          cout << "Sintaksna greska na liniji " << lineNumber << "Operand nije dobro definisan" << endl;
          exit(-1);
        }
      }

      if (lineCorrect == false)
      {
        cout << "SINTAKSNA GRESKA NA LINIJI " << lineNumber << " Ne postoji naredba ni direktiva ovog formata!" << endl;
        exit(-1);
      }
      // Printing the token vector

      // for (int i = 0; i < tokens.size(); i++)
      // {
      //   cout << tokens[i] << '\n';
      // }
    }
    lineNumber++;
  }
  printSymbolTable(symbol_table, out);
}
void secondPass(ifstream &f, std::ofstream &out)
{
  lineNumber = 1;
  // cout << "Pocinje drugi prolaz" << endl;
  // cout << "-------------------------------" << endl;
  while (getline(f, line))
  {

    vector<string> tokens;
    skipNum = 0;
    stringstream check1(line);
    string intermediate;
    comment = false;
    // Tokenizing of current line using space

    while (getline(check1, intermediate, ' '))
    {
      intermediate.erase(std::remove_if(intermediate.begin(), intermediate.end(), ::isspace), intermediate.end());
      if (intermediate == "#" || intermediate.substr(0, 1) == "#")
        comment = true;
      if (intermediate != "" && intermediate != "\t" && intermediate != " " && !comment)
      {
        tokens.push_back(intermediate);
      }
    }
    if (tokens.size() == 0) /*ignore this line */
      cout << "";
    // ---------------------------------------------------------------------------------------------------------------
    else
    {
      // .section DIRECTIVE PROCESSING
      if (regex_match(tokens[0], match, sectionRegex) && tokens.size() == 2)
      {

        for (it = symbol_table.begin(); it != symbol_table.end(); it++)
        {
          if (it->label == tokens[1])
          {
            sectionContent.push_back("");
            break;
          }
        }
        CURRENTSECTION = tokens[1];
        LOCATIONCOUNTER = 0;
      }

      if (tokens.size() == 3 && tokens[1] == ".section" && regex_match(tokens[0], match, labelRegex))
      {
        for (it = symbol_table.begin(); it != symbol_table.end(); it++)
        {
          if (it->label == tokens[2])
          {
            sectionContent.push_back("");
            break;
          }
        }
        CURRENTSECTION = tokens[2];
        LOCATIONCOUNTER = 0;
      }

      //----------------------------------------------------------------------------------------------------------------

      // .word DIRECTIVE PROCESSING
      if (regex_match(tokens[0], match, wordRegex) && tokens.size() >= 2)
      {
        status = checkWord(0, tokens);
        if (status == 404)
          exit(-1);
      }
      if (regex_match(tokens[0], match, labelRegex) && tokens.size() >= 3 && tokens[1] == ".word")
      {
        status = checkWord(1, tokens);
        if (status == 404)
          exit(-1);
      }

      // //----------------------------------------------------------------------------------------------------------------

      // .skip DIRECTIVE PROCESSING
      if (tokens[0] == ".skip" && tokens.size() == 2)
      {
        if (tokens[1].substr(0, 1) != "0")
        {
          skipNum = (short)stoi(tokens[1]);
        }
        else if (tokens[1].substr(0, 2) == "0x")
        {
          skipNum = (short)stoi(tokens[1], 0, 16);
        }
        else if (tokens[1].substr(0, 1) == "0")
        {
          skipNum = (short)stoi(tokens[1], 0, 8);
        }
      }
      if (regex_match(tokens[0], match, labelRegex) && tokens.size() == 3 && tokens[1] == ".skip")
      {
        if (tokens[2].substr(0, 1) != "0")
        {
          skipNum = (short)stoi(tokens[2]);
        }
        else if (tokens[2].substr(0, 2) == "0x")
        {
          skipNum = (short)stoi(tokens[2], 0, 16);
        }
        else if (tokens[2].substr(0, 1) == "0")
        {
          skipNum = (short)stoi(tokens[2], 0, 8);
        }
      }

      // cout << " SKIPNUM NA LINIJI " << lineNumber << " : " << skipNum << endl;
      for (int i = 0; i < skipNum; i++)
      {
        sectionContent[sectionContent.size() - 1].append("00");
        if (i == skipNum - 1)
          sectionContent[sectionContent.size() - 1].append(" ");
      }
      LOCATIONCOUNTER += skipNum;
      GLC += skipNum;

      //----------------------------------------------------------------------------------------------------------------
      // (halt, ret, iret) PROCESSING
      if (regex_match(tokens[0], match, oneByteCommandsRegex) && tokens.size() == 1)
      {

        if (tokens[0] == "halt")
          sectionContent[sectionContent.size() - 1].append("00 ");
        if (tokens[0] == "iret")
          sectionContent[sectionContent.size() - 1].append("20 ");
        if (tokens[0] == "ret")
          sectionContent[sectionContent.size() - 1].append("40 ");
        LOCATIONCOUNTER += 1;
        GLC += 1;
      }

      if (regex_match(tokens[0], match, labelRegex) && tokens.size() == 2 && regex_match(tokens[1], match, oneByteCommandsRegex))
      {
        if (tokens[1] == "halt")
          sectionContent[sectionContent.size() - 1].append("00 ");
        if (tokens[1] == "iret")
          sectionContent[sectionContent.size() - 1].append("20 ");
        if (tokens[1] == "ret")
          sectionContent[sectionContent.size() - 1].append("40 ");
        LOCATIONCOUNTER += 1;
        GLC += 1;
      }
      // //----------------------------------------------------------------------------------------------------------------
      // // (int, xchg, add, sub, mul, div, cmp, not, and, or, xor, test, shl, shr) PROCESSING
      if (regex_match(tokens[0], match, twoByteCommandsRegex) ||
          (tokens.size() >= 3 && regex_match(tokens[0], match, labelRegex) && regex_match(tokens[1], match, twoByteCommandsRegex)))
      {

        int i = (regex_match(tokens[0], match, twoByteCommandsRegex)) ? 0 : 1;

        if (tokens[i] == "int" || tokens[i] == "not") // int Processing
        {
          string num = "";
          if (tokens[i + 1] == "psw")
            num = "8";
          else if (tokens[i + 1] == "pc")
            num = "7";
          else if (tokens[i + 1] == "sp")
            num = "6";
          else
            num = tokens[i + 1].substr(1, 1);

          if (tokens[i] == "int")
            sectionContent[sectionContent.size() - 1].append("10").append(num).append("f ");
          if (tokens[i] == "not")
            sectionContent[sectionContent.size() - 1].append("80").append(num).append("f ");
        }
        else
        { // 2 byte instruction, but not int or not should be this else branch
          string regs = "";
          for (int j = i + 1; j < tokens.size(); j++)
          {
            regs.append(tokens[j]);
          }

          regex_match(regs, match, regsRegex);
          string num1;
          string num2;
          string regD = match[1].str();
          string regS = match[2].str();

          if (regD == "psw")
            num1 = "8";
          else if (regD == "pc")
            num1 = "7";
          else if (regD == "sp")
            num1 = "6";
          else
            num1 = regD.substr(1, 1);

          if (regS == "psw")
            num2 = "8";
          else if (regS == "pc")
            num2 = "7";
          else if (regS == "sp")
            num2 = "6";
          else
            num2 = regS.substr(1, 1);

          if (tokens[i] == "xchg")
            sectionContent[sectionContent.size() - 1].append("60").append(num1).append(num2);
          if (tokens[i] == "add")
            sectionContent[sectionContent.size() - 1].append("70").append(num1).append(num2);
          if (tokens[i] == "sub")
            sectionContent[sectionContent.size() - 1].append("71").append(num1).append(num2);
          if (tokens[i] == "mul")
            sectionContent[sectionContent.size() - 1].append("72").append(num1).append(num2);
          if (tokens[i] == "div")
            sectionContent[sectionContent.size() - 1].append("73").append(num1).append(num2);
          if (tokens[i] == "cmp")
            sectionContent[sectionContent.size() - 1].append("74").append(num1).append(num2);
          if (tokens[i] == "and")
            sectionContent[sectionContent.size() - 1].append("81").append(num1).append(num2);
          if (tokens[i] == "or")
            sectionContent[sectionContent.size() - 1].append("82").append(num1).append(num2);
          if (tokens[i] == "xor")
            sectionContent[sectionContent.size() - 1].append("83").append(num1).append(num2);
          if (tokens[i] == "test")
            sectionContent[sectionContent.size() - 1].append("84").append(num1).append(num2);
          if (tokens[i] == "shl")
            sectionContent[sectionContent.size() - 1].append("90").append(num1).append(num2);
          if (tokens[i] == "shr")
            sectionContent[sectionContent.size() - 1].append("91").append(num1).append(num2);

          sectionContent[sectionContent.size() - 1].append(" ");
        }

        LOCATIONCOUNTER += 2;
        GLC += 2;
      }
      // //----------------------------------------------------------------------------------------------------------------
      // // (push, pop) PROCESSING
      if (regex_match(tokens[0], match, threeByteCommandRegex) ||
          (regex_match(tokens[0], match, labelRegex) && tokens.size() > 2 && regex_match(tokens[1], match, threeByteCommandRegex)))
      {
        int i = (regex_match(tokens[0], match, threeByteCommandRegex)) ? 0 : 1;

        string num;
        if (tokens[i + 1] == "psw")
          num = "8";
        else if (tokens[i + 1] == "pc")
          num = "7";
        else if (tokens[i + 1] == "sp")
          num = "6";
        else
          num = tokens[i + 1].substr(1, 1);

        if (tokens[i] == "push")
          sectionContent[sectionContent.size() - 1].append("b0").append(num).append("612 ");
        if (tokens[i] == "pop")
          sectionContent[sectionContent.size() - 1].append("a0").append(num).append("642 ");

        LOCATIONCOUNTER += 3;
        GLC += 3;
      }
      // //--------------------------------------------------------------------------------------------------------------
      // //(call, jmp, jeq, jne, jgt) PROCESSING
      if (regex_match(tokens[0], match, jumpsRegex) ||
          (regex_match(tokens[0], match, labelRegex) && tokens.size() > 2 && regex_match(tokens[1], match, jumpsRegex)))
      {
        int i = (regex_match(tokens[0], match, jumpsRegex)) ? 0 : 1;
        string operand = "";
        for (int j = i + 1; j < tokens.size(); j++)
        {
          operand.append(tokens[j]);
        }

        if (tokens[i] == "call")
          sectionContent[sectionContent.size() - 1].append("30f");
        if (tokens[i] == "jmp")
          sectionContent[sectionContent.size() - 1].append("50f");
        if (tokens[i] == "jeq")
          sectionContent[sectionContent.size() - 1].append("51f");
        if (tokens[i] == "jne")
          sectionContent[sectionContent.size() - 1].append("52f");
        if (tokens[i] == "jgt")
          sectionContent[sectionContent.size() - 1].append("53f");

        if (regex_match(operand, match, jumps3Regex)) // *reg and *[reg] processing
        {

          string num = match[1].str().substr(1, 1);
          if (match[0].str().substr(1, 1) == "[") // reg ind
          {
            sectionContent[sectionContent.size() - 1].append(num).append("02 ");
          }
          else //reg
          {
            sectionContent[sectionContent.size() - 1].append(num).append("f1 ");
          }

          LOCATIONCOUNTER += 3;
          GLC += 3;
        }

        else if (regex_match(operand, match, jumpsLiteral) || regex_match(operand, match, jumpsSymbol) || regex_match(operand, match, jumpsRegSymbol) || regex_match(operand, match, jumpsRegNumber))
        {
          //5 bit jumps processing
          if (regex_match(operand, match, jumpsSymbol)) //jump <operand> where operand is symbol
          {
            string symb = operand;
            if (symb.substr(0, 1) == "%")
            {
              // PC REL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
              symb.erase(0, 1);
              sectionContent[sectionContent.size() - 1].append("705");

              // match[3] is stripped down operand
              int addend;
              for (it = symbol_table.begin(); it != symbol_table.end(); it++)
              {
                if (it->label == match[3].str()) //looking for symbol in symbol_table
                {
                  if (it->local == 0 || it->section == "ABS") // global or extern symbol encountered (TREBA DA POPRAVIS OBRADU GLOBAL, EXTERN I WORD
                  {
                    sectionContent[sectionContent.size() - 1].append("fffe ");
                    if (it->section == "ABS")
                    {
                      relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER + 3, "R_386_PC16_ABS", it->ordNumber, -2));
                    }
                    else
                      relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER + 3, "R_386_PC16", it->ordNumber, -2));
                  }
                  else // local symbol encountered
                  {
                    // STA AKO JE EQU SIMBOL ( vrv se gledaju kao globalni simboli, proveri !!!!!!!!!!!!!!!!!!!!!!!!)
                    if (it->section == CURRENTSECTION) // local symbol from the same section
                    {
                      int value = it->offset - (LOCATIONCOUNTER + 5);
                      std::stringstream sstream;

                      sstream << std::hex << setfill('0') << setw(4) << value;

                      string result = sstream.str();
                      if (value < 0)
                        result = result.substr(4, 4);

                      sectionContent[sectionContent.size() - 1].append(result);
                      sectionContent[sectionContent.size() - 1].append(" ");
                    }
                    else //local symbol from different section
                    {
                      int value = it->offset - 2;
                      std::stringstream sstream;
                      sstream << std::hex << setfill('0') << setw(4) << value;
                      string result = sstream.str();
                      sectionContent[sectionContent.size() - 1].append(result);
                      sectionContent[sectionContent.size() - 1].append(" ");

                      int ordSec;
                      list<symbol>::iterator it1;
                      for (it1 = symbol_table.begin(); it1 != symbol_table.end(); it1++)
                      {

                        if (it1->label == it->section)
                        {
                          ordSec = it1->ordNumber;
                          break;
                        }
                      }
                      relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER + 3, "R_386_PC16", ordSec, value));
                    }
                  }
                  break;
                }
              }
            }

            else // symbol absolute or symbol memory
            {
              if (symb.substr(0, 1) == "*")
              {
                symb.erase(0, 1);
                sectionContent[sectionContent.size() - 1].append("f04");
              }
              else
              {
                sectionContent[sectionContent.size() - 1].append("f00");
              }
              int payload;
              for (it = symbol_table.begin(); it != symbol_table.end(); it++)
              {
                string currentSymbol = it->label;
                if (currentSymbol == symb /*&& it->section=="UND"*/)
                {
                  list<symbol>::iterator it1;
                  int secOrdNum;
                  if (it->section != "ABS")
                  {
                    for (it1 = symbol_table.begin(); it1 != symbol_table.end(); it1++)
                    {
                      if (it1->label == it->section)
                      {
                        secOrdNum = it1->ordNumber;
                        break;
                      }
                    }
                  }
                  else
                  {
                    // cout << " NE BI TREBALO DA PRAVIS RELOKACIONI ZAPIS OVDE (POPRAVIIIIIIIIIIIIIIIIIII) " << lineNumber << " JER JE EQU SIMBOL" << endl;
                    // cout << "SAMO if apsolutnaSekcija zatvori skroz dole na dnu" << endl;
                    secOrdNum = it->ordNumber;
                  }
                  payload = it->offset;
                  std::stringstream sstream;
                  sstream << std::hex << setfill('0') << setw(4) << payload;
                  string result = sstream.str();
                  sectionContent[sectionContent.size() - 1].append(result);
                  sectionContent[sectionContent.size() - 1].append(" ");
                  // MORAS DA OBRADIS SLUCAJ ZA APSOLUTNU SEKCIJU
                  if (it->section == "UND")
                    secOrdNum = it->ordNumber;
                  if (it->section != "ABS")
                    relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER + 3, "R_386_16", secOrdNum, 0));
                  break;
                }
              }
            }
          }

          if (regex_match(operand, match, jumpsLiteral))
          {
            string num = operand;
            if (match[i].str().substr(0, 1) == "*") // memory (literal)
            {
              sectionContent[sectionContent.size() - 1].append("f04");
              num.erase(0, 1);
            }
            else // literal
            {
              sectionContent[sectionContent.size() - 1].append("f00");
            }

            short value;
            if (num.substr(0, 1) != "0")
            {
              value = (short)stoi(num);
            }
            else if (num.substr(0, 2) == "0x")
            {
              value = (short)stoi(num, 0, 16);
            }
            else if (num.substr(0, 1) == "0")
            {
              value = (short)stoi(num, 0, 8);
            }
            if (value < 0)
            {
              cout << " SKOK NA NEGATIVNU ADRESU NA LINIJI " << lineNumber << endl;
              exit(-1);
            }
            std::stringstream sstream;
            sstream << std::hex << setfill('0') << setw(4) << value;
            string result = sstream.str();
            sectionContent[sectionContent.size() - 1].append(result);
            sectionContent[sectionContent.size() - 1].append(" ");

            // int value = stoi(num);
            // std::stringstream sstream;
            // sstream << std::hex << setfill('0') << setw(4) << value;
            // string result = sstream.str();
            // sectionContent[sectionContent.size() - 1].append(result);
            // sectionContent[sectionContent.size() - 1].append(" ");
          }

          if (regex_match(operand, match, jumpsRegNumber))
          {
            // for (int i = 0; i < match.size(); i++)
            // {
            //   // cout << match[i].str() << endl;
            // }
            string num = "";
            if (match[1].str() == "psw")
              num = "8";
            else if (match[1].str() == "pc")
              num = "7";
            else if (match[1].str() == "sp")
              num = "6";
            else
              num = match[1].str().substr(1, 1);

            sectionContent[sectionContent.size() - 1].append(num);
            sectionContent[sectionContent.size() - 1].append("03");

            short value;
            if (match[2].str().substr(0, 1) != "0")
            {
              value = (short)stoi(match[2].str());

              std::stringstream sstream;
              sstream << std::hex << setfill('0') << setw(4) << value;
              string result = sstream.str();
              sectionContent[sectionContent.size() - 1].append(result);
              sectionContent[sectionContent.size() - 1].append(" ");
            }
            else if (match[2].str().substr(0, 2) == "0x")
            {
              value = (short)stoi(match[2].str(), 0, 16);

              std::stringstream sstream;
              sstream << std::hex << setfill('0') << setw(4) << value;
              string result = sstream.str();
              sectionContent[sectionContent.size() - 1].append(result);
              sectionContent[sectionContent.size() - 1].append(" ");
            }
            else if (match[2].str().substr(0, 1) == "0")
            {
              value = (short)stoi(match[2].str(), 0, 8);
              std::stringstream sstream;
              sstream << std::hex << setfill('0') << setw(4) << value;
              string result = sstream.str();
              sectionContent[sectionContent.size() - 1].append(result);
              sectionContent[sectionContent.size() - 1].append(" ");
            }
          }

          if (regex_match(operand, match, jumpsRegSymbol))
          {
            // cout << "MORAMO ODRADITI ONU PROVERU ZA Symbol DEO!!" << endl;
            // for (int i = 0; i < match.size(); i++)
            // {
            //   cout << match[i].str() << endl;
            // }

            string num = "";
            if (match[2].str() == "psw")
              num = "8";
            else if (match[2].str() == "pc")
              num = "7";
            else if (match[2].str() == "sp")
              num = "6";
            else
              num = match[2].str().substr(1, 1);

            sectionContent[sectionContent.size() - 1].append(num).append("03");

            int payload;
            for (it = symbol_table.begin(); it != symbol_table.end(); it++)
            {
              string currentSymbol = it->label;
              if (currentSymbol == match[3].str() /*&& it->section=="UND"*/)
              {
                list<symbol>::iterator it1;
                int secOrdNum;

                if (it->section != "ABS" && it->local == 1)
                {
                  for (it1 = symbol_table.begin(); it1 != symbol_table.end(); it1++)
                  {
                    if (it1->label == it->section)
                    {
                      secOrdNum = it1->ordNumber;
                      break;
                    }
                  }
                }
                else if (it->local == 0)
                {
                  secOrdNum = it->ordNumber;
                }
                // else
                // {
                //   cout << " NE BI TREBALO DA PRAVIS RELOKACIONI ZAPIS OVDE " << lineNumber << " JER JE EQU SIMBOL" << endl;
                //   cout << "SAMO if apsolutnaSekcija zatvori skroz dole na dnu" << endl;
                //   secOrdNum = it->ordNumber;
                // }
                payload = it->offset;
                std::stringstream sstream;
                sstream << std::hex << setfill('0') << setw(4) << payload;
                string result = sstream.str();
                sectionContent[sectionContent.size() - 1].append(result);
                sectionContent[sectionContent.size() - 1].append(" ");
                // MORAS DA OBRADIS SLUCAJ ZA APSOLUTNU SEKCIJU
                if (it->section != "ABS")
                  relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER + 3, "R_386_16", secOrdNum, 0));
                break;
              }
            }
          }
          LOCATIONCOUNTER += 5;
          GLC += 5;
        }

        // else
        // {
        //   cout << "Sintaksna greska na liniji " << lineNumber << "Operand nije dobro definisan" << endl;
        //   exit(-1);
        // }
      }
      // //--------------------------------------------------------------------------------------------------------------
      // // (ldr, str) PROCESSING
      if (regex_match(tokens[0], match, memRegex) ||
          (regex_match(tokens[0], match, labelRegex) && tokens.size() > 2 && regex_match(tokens[1], match, memRegex)))
      {
        int i = (regex_match(tokens[0], match, memRegex)) ? 0 : 1;
        if (tokens[i] == "ldr")
          sectionContent[sectionContent.size() - 1].append("a0");
        if (tokens[i] == "str")
          sectionContent[sectionContent.size() - 1].append("b0");

        string operand = "";
        for (int j = i + 1; j < tokens.size(); j++)
        {
          operand.append(tokens[j]);
        }

        if (regex_match(operand, match, mem3Regex))
        {
          // for (int i = 0; i < match.size(); i++)
          // {
          //   cout << match[i].str() << endl;
          // }
          string num1 = "";
          if (match[1].str() == "psw")
            num1 = "8";
          else if (match[1].str() == "pc")
            num1 = "7";
          else if (match[1].str() == "sp")
            num1 = "6";
          else
            num1 = match[1].str().substr(1, 1);

          sectionContent[sectionContent.size() - 1].append(num1);

          if (match[2].str().substr(0, 1) != "[")
          { //reg dir

            string num2 = "";
            if (match[2].str() == "psw")
              num2 = "8";
            else if (match[2].str() == "pc")
              num2 = "7";
            else if (match[2].str() == "sp")
              num2 = "6";
            else
              num2 = match[2].str().substr(1, 1);

            sectionContent[sectionContent.size() - 1].append(num2).append("01 ");
          }
          else //reg ind
          {
            string num2 = "";
            if (match[5].str() == "psw")
              num2 = "8";
            else if (match[5].str() == "pc")
              num2 = "7";
            else if (match[5].str() == "sp")
              num2 = "6";
            else
              num2 = match[5].str().substr(1, 1);

            sectionContent[sectionContent.size() - 1].append(num2).append("02 ");
          }
          LOCATIONCOUNTER += 3;
          GLC += 3;
        }

        else if (regex_match(operand, match, mem5Literal) || regex_match(operand, match, memSymbol) || regex_match(operand, match, memRegSymbol) || regex_match(operand, match, memRegNumber))
        {
          if (regex_match(operand, match, mem5Literal))
          {

            string num = "";
            if (match[1].str() == "psw")
              num = "8";
            else if (match[1].str() == "pc")
              num = "7";
            else if (match[1].str() == "sp")
              num = "6";
            else
              num = match[1].str().substr(1, 1);

            sectionContent[sectionContent.size() - 1].append(num).append("f");
            if (match[2].str().substr(0, 1) == "$")
            { // literal , use match[3]

              sectionContent[sectionContent.size() - 1].append("00");

              // cout << " MATCH  2 " << match[2].str() << " lajna " << lineNumber << endl;

              short value;
              if (match[2].str().substr(1, 2) == "0x")
              {
                string hexa = match[2].str();
                hexa = hexa.erase(0, 1);
                value = (short)stoi(hexa, 0, 16);
                // cout << "VALUE " << value << endl;
              }
              else if (match[2].str().substr(1, 1) != "0")
              {
                value = (short)stoi(match[3].str());
              }
              else
              {
                string octa = match[2].str();
                octa = octa.erase(0, 1);
                value = (short)stoi(octa, 0, 8);
              }
              std::stringstream sstream;
              sstream << std::hex << setfill('0') << setw(4) << value;
              string result = sstream.str();
              sectionContent[sectionContent.size() - 1].append(result);
              sectionContent[sectionContent.size() - 1].append(" ");
            }
            else
            {
              //mem adressing
              sectionContent[sectionContent.size() - 1].append("04");

              short value;
              if (match[2].str().substr(0, 1) != "0")
              {
                value = (short)stoi(match[2].str());
              }
              else if (match[2].str().substr(0, 2) == "0x")
              {
                value = (short)stoi(match[2].str(), 0, 16);
              }
              else if (match[2].str().substr(0, 1) == "0")
              {
                value = (short)stoi(match[2].str(), 0, 8);
              }
              std::stringstream sstream;
              sstream << std::hex << setfill('0') << setw(4) << value;
              string result = sstream.str();
              sectionContent[sectionContent.size() - 1].append(result);
              sectionContent[sectionContent.size() - 1].append(" ");
            }
          }
          if (regex_match(operand, match, memSymbol))
          {
            string num1 = "";
            if (match[1].str() == "psw")
              num1 = "8";
            else if (match[1].str() == "pc")
              num1 = "7";
            else if (match[1].str() == "sp")
              num1 = "6";
            else
              num1 = match[1].str().substr(1, 1);

            sectionContent[sectionContent.size() - 1].append(num1);
            bool pcrel = false;

            if (match[3].str() == "$" || match[3].str() == "" || match[3].str() == "%")
            { // value of symbol, relocation possible
              if (match[3].str() == "$")
                sectionContent[sectionContent.size() - 1].append("f00");
              if (match[3].str() == "")
              {
                sectionContent[sectionContent.size() - 1].append("f04");
              }
              if (match[3].str() == "%")
              {
                pcrel = true;
                sectionContent[sectionContent.size() - 1].append("703");
              }

              short payload;
              bool abs = false;
              for (it = symbol_table.begin(); it != symbol_table.end(); it++)
              {
                if (it->label == match[4].str())
                {
                  if (it->section == "ABS")
                    abs = true;
                  payload = it->offset;
                  if (pcrel)
                    payload -= 2;
                  int ornum = it->ordNumber;
                  string reltype = pcrel ? "R_386_PC16" : "R_386_16";
                  if (it->local == 1 && it->section != "ABS")
                  {
                    list<symbol>::iterator it1;
                    for (it1 = symbol_table.begin(); it1 != symbol_table.end(); it1++)
                    {
                      if (it1->label == it->section)
                      {
                        ornum = it1->ordNumber;
                        break;
                      }
                    }
                  }
                  if (abs && pcrel)
                  {
                    relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER + 3, "R_386_PC16_ABS", ornum, 0));
                    payload = -2;
                  }
                  if (!abs)
                  {
                    relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER + 3, reltype, ornum, 0));
                  }

                  break;
                }
              }

              std::stringstream sstream;
              sstream << std::hex << setfill('0') << setw(4) << payload;
              string result = sstream.str();
              sectionContent[sectionContent.size() - 1].append(result);
              sectionContent[sectionContent.size() - 1].append(" ");
            }
          }

          if (regex_match(operand, match, memRegSymbol))
          { // [reg + simbol]

            string num1 = "";
            if (match[1].str() == "psw")
              num1 = "8";
            else if (match[1].str() == "pc")
              num1 = "7";
            else if (match[1].str() == "sp")
              num1 = "6";
            else
              num1 = match[1].str().substr(1, 1);

            string num2 = "";
            if (match[3].str() == "psw")
              num2 = "8";
            else if (match[3].str() == "pc")
              num2 = "7";
            else if (match[3].str() == "sp")
              num2 = "6";
            else
              num2 = match[3].str().substr(1, 1);

            sectionContent[sectionContent.size() - 1].append(num1 + num2 + "03");

            short payload;
            bool abs = false;
            for (it = symbol_table.begin(); it != symbol_table.end(); it++)
            {
              if (it->label == match[4].str())
              {
                if (it->section == "ABS")
                  abs = true;
                payload = it->offset;
                int ornum = it->ordNumber;
                if (it->local == 1 && it->section != "ABS")
                {
                  list<symbol>::iterator it1;
                  for (it1 = symbol_table.begin(); it1 != symbol_table.end(); it1++)
                  {
                    if (it1->label == it->section)
                    {
                      ornum = it1->ordNumber;
                      break;
                    }
                  }
                }
                if (!abs)
                  relocation_table.push_back(relocation(CURRENTSECTION, LOCATIONCOUNTER + 3, "R_386_16", ornum, 0));
                break;
              }
            }

            std::stringstream sstream;
            sstream << std::hex << setfill('0') << setw(4) << payload;
            string result = sstream.str();
            sectionContent[sectionContent.size() - 1].append(result);
            sectionContent[sectionContent.size() - 1].append(" ");
          }

          if (regex_match(operand, match, memRegNumber))
          { // [reg + simbol]

            string num1 = "";
            if (match[1].str() == "psw")
              num1 = "8";
            else if (match[1].str() == "pc")
              num1 = "7";
            else if (match[1].str() == "sp")
              num1 = "6";
            else
              num1 = match[1].str().substr(1, 1);

            string num2 = "";
            if (match[3].str() == "psw")
              num2 = "8";
            else if (match[3].str() == "pc")
              num2 = "7";
            else if (match[3].str() == "sp")
              num2 = "6";
            else
              num2 = match[3].str().substr(1, 1);

            sectionContent[sectionContent.size() - 1].append(num1 + num2 + "03");

            string sabirak = match[4].str();
            short payload = 0;

            if (sabirak.substr(0, 2) == "0x")
            {
              payload = (short)stoi(sabirak, 0, 16);
            }
            else
            {
              payload = (short)stoi(sabirak);
            }

            std::stringstream sstream;
            sstream << std::hex << setfill('0') << setw(4) << payload;
            string result = sstream.str();
            sectionContent[sectionContent.size() - 1].append(result);
            sectionContent[sectionContent.size() - 1].append(" ");
          }

          LOCATIONCOUNTER += 5;
          GLC += 5;
        }
      }
    }
    lineNumber++;
  }

  // out << "CODE: " << endl;

  // COUT FOR GOOD VIEW OF INSTRUCTIONS
  // for (int i = 0; i < sectionContent.size(); i++)
  // {

  //   out << sections[i] << ":" << sectionContent[i] << endl;

  // }

  for (int i = 0; i < sectionContent.size(); i++)
  {
    out << endl;
    out << sections[i] << endl;
    int gc = 0;
    int cnt = 0;
    out << setw(4) << gc << " : ";
    for (int j = 0; j < sectionContent[i].size(); j++)
    {
      if (sectionContent[i][j] != ' ')
      {
        out << sectionContent[i][j];
        cnt++;
      }
      if (cnt > 0 && cnt % 2 == 0 && sectionContent[i][j + 1] != ' ')
      {
        out << " ";
      }
      if (cnt == 40)
      {
        out << endl;
        gc += 20;
        out << setw(4) << gc << " : ";
        cnt = 0;
      }
    }
  }
  out << endl;
}
int main(int argc, char const *argv[])
{

  std::ofstream out(argv[2]);
  ifstream fajl(argv[1]);
  ifstream fajl1(argv[1]);
  first_Pass = true;
  firstPass(fajl, out);
  first_Pass = false;
  LOCATIONCOUNTER = GLC = 0;
  secondPass(fajl1, out);
  out << "-------------------------------------------------------------------------------------------------------------" << endl;
  if (relocation_table.size() > 0)
    printRelocationTable(relocation_table, out);

  if (absoluteSection.size() > 0)
  {
    out << "-------------------------------------------------------------------------------------------------------------" << endl;
    out << " ABS : " << absoluteSection << endl;
    out << "-------------------------------------------------------------------------------------------------------------" << endl;
  }
  return 0;
}