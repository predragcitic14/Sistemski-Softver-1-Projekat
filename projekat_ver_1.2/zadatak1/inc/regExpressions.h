#ifndef REGEXPRESSIONS_H_
#define REGEXPRESSIONS_H_

#include <regex>
using namespace std;

regex varReg("([a-z]|[A-Z])[a-z|A-Z|0-9|_]*");
regex labelRegex("[\t+| +]*([a-z]|[A-Z])[a-z|A-Z|0-9|_]*:[\t+| +]*");

regex directiveRegex("[\t+| +]*(.skip|.word|.section|.extern|.global|.equ|.end)[\t+| +]*");
regex skipRegex("[\t+| +]*(.skip)[\t+| +]*");
regex wordRegex("[\t+| +]*(.word)[\t+| +]*");
regex sectionRegex("[\t+| +]*(.section)[\t+| +]*");
regex externRegex("[\t+| +]*(.extern)[\t+| +]*");
regex globalRegex("(.global|.extern)");
// regex equRegex("[\t+| +]*(.equ)[\t+| +]*");
regex endRegex("[\t+| +]*(.end)[\t+| +]*");
regex numberRegex("([1-9][0-9]*)|(0x[A-F|a-f|0-9]{1,4})|(0[0-7]*)");
regex commentRegex("[\t+| +]*(#)[\t+| +]*");

regex oneByteCommandsRegex("(halt|iret|ret)");
regex twoByteCommandsRegex("(int|xchg|add|sub|mul|div|cmp|not|and|or|xor|test|shl|shr)");
regex threeByteCommandRegex("(push|pop)");
regex jumpsRegex("(call|jmp|jeq|jne|jgt)");
regex memRegex("(ldr|str)");

regex registerRegex("((r[0-7])|psw|sp|pc)");
regex regsRegex("(r[0-7]|psw|sp|pc),(r[0-7]|psw|sp|pc)");

regex equRegex("([a-z]|[A-Z])[a-z|A-Z|0-9|_]*,(([1-9][0-9]*)|(0x[A-F|a-f|0-9]{1,4})|(0[0-7]*))");
//PRE PROMENE
regex jumps3Regex("\\*[\\[]{0,1}(r[0-7]|pc|sp|psw)[\\]]{0,1}");
regex jumpsLiteral("(\\*{0,1}(([1-9][0-9]*)|(0x[A-F|a-f|0-9]{1,4})|(0[0-7]*)))");
regex jumpsSymbol("((\\*{0,1}|%{0,1}|)(([a-z]|[A-Z])[a-z|A-Z|0-9|_]*))");
regex jumpsRegSymbol("(\\*\\[(r[0-7]|psw|pc|sp)\\+(([a-z]|[A-Z])[a-z|A-Z|0-9|_]*)\\])");
regex jumpsRegNumber("\\*\\[(r[0-7]|psw|pc|sp)\\+(([1-9][0-9]*)|(0x[A-F|a-f|0-9]{1,4})|(0[0-7]*))\\]");

regex mem3Regex("(r[0-7]|psw|sp|pc),((r[0-7]|psw|sp|pc)|(\\[(r[0-7]|psw|sp|pc)\\]))"); //PROMENI DA OVAKO IZGLEDA SVAKA SA {0,1}
regex mem5Literal("(r[0-7]|psw|sp|pc),(\\${0,1}([1-9][0-9]*)|(\\${0,1}0x[A-F|a-f|0-9]{1,4})|(\\${0,1}0[0-7]*))");
regex memSymbol("(r[0-7]|psw|sp|pc),((\\${0,1}|%{0,1}|)(([a-z]|[A-Z])[a-z|A-Z|0-9|_]*))");
regex memRegSymbol("(r[0-7]|psw|sp|pc),(\\[(r[0-7]|pc|sp|psw)\\+(([a-z]|[A-Z])[a-z|A-Z|0-9|_]*)\\])");
regex memRegNumber("(r[0-7]|psw|sp|pc),(\\[(r[0-7]|pc|sp|psw)\\+([0-9]+|0x[A-F|a-f|0-9]{1,4})\\])");

#endif