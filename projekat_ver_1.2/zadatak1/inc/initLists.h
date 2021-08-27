#ifndef INITLISTS_H_
#define INITLISTS_H_

#include <cstdlib>
#include <iostream>
#include <bits/stdc++.h>

using namespace std;

vector<string> initializeDirective(vector<string> myVec)
{
    myVec.push_back(".global");
    myVec.push_back(".extern");
    myVec.push_back(".section");
    myVec.push_back(".word");
    myVec.push_back(".skip");
    myVec.push_back(".equ");
    myVec.push_back(".end");
    return myVec;
}

vector<string> initializeCommands(vector<string> commands)
{
    // commands.push_back("halt");     // :: 0x00
    //commands.push_back("int"); // :: 0x10 (regD)F :: + regD  :: 2 bajta
    // commands.push_back("iret");     // :: 0x20
    commands.push_back("call"); // :: 0x30 F(S) (U)(A)  :: + operand :: 3 | 5 bajtova
    // commands.push_back("ret");      // :: 0x40
    commands.push_back("jmp"); // :: 0x50 :: + operand :: 3 | 5 bajtova
    commands.push_back("jeq"); // :: 0x51 :: + operand :: 3 | 5 bajtova
    commands.push_back("jne"); // :: 0x52 :: + operand :: 3 | 5 bajtova
    commands.push_back("jgt"); // :: 0x53 :: + operand :: 3 | 5 bajtova
    // commands.push_back("push"); // + regD  :: 3 bajta
    // commands.push_back("pop");  // + regD  :: 3 bajta
    // commands.push_back("xchg"); // :: 0x60 :: + RegD, regS :: 2 bajta
    // commands.push_back("add");  // :: 0x70 :: + RegD, regS :: 2 bajta
    // commands.push_back("sub");  // :: 0x71 :: + RegD, regS :: 2 bajta
    // commands.push_back("mul");  // :: 0x72 :: + RegD, regS :: 2 bajta
    // commands.push_back("div");  // :: 0x73 :: + RegD, regS :: 2 bajta
    // commands.push_back("cmp");  // :: 0x74 :: + RegD, regS :: 2 bajta
    // commands.push_back("not");  // :: 0x80 :: + regD       :: 2 bajta
    // commands.push_back("and");  // :: 0x81 :: + RegD, regS :: 2 bajta
    // commands.push_back("or");   // :: 0x82 :: + RegD, regS :: 2 bajta
    // commands.push_back("xor");  // :: 0x83 :: + RegD, regS :: 2 bajta
    // commands.push_back("test"); // :: 0x84 :: + RegD, regS :: 2 bajta
    // commands.push_back("shl");  // :: 0x90 :: + RegD, regS :: 2 bajta
    // commands.push_back("shr");  // :: 0x91 :: + RegD, regS :: 2 bajta
    commands.push_back("ldr"); // :: 0xA0 :: + regD, operand :: 3|5 bajtova
    commands.push_back("str"); // :: 0xB0 :: + regD, operand :: 3|5 bajtova
    return commands;
}

void printVector(vector<string> l)
{
    for (int i = 0; i < l.size(); i++)
    {
        cout << l[i] << endl;
    }
}

#endif