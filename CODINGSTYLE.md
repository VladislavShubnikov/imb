- [Introduction](#introduction)
- [Rules](#rules)
  * [Header files](#header-files)
  * [Scoping](#scoping)
  * [Classes](#classes)
  * [Functions](#functions)
  * [Smart pointers](#smart-pointers)
  * [Other](#other)
  * [early-return-rule](#early-return-rule)
  * [goto-rule](#goto-rule)
- [Inclusive language](#inclusive-language)
- [Naming](#naming)
- [Formatting](#formatting)
- [Logging levels are Qt native](#logging-levels-are-qt-native)
- [Qt using rules](#qt-using-rules)

## Introduction

The coding style document has a goal to manage general approach in C++ code writing inthe project. 
Coding style and formatting are pretty arbitrary, but a project is much easier to follow if everyone uses the same style.
Tha main part of it is based on [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and
[C++ Core Guidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rp-direct). It is organized
as a list of rules. Some of the rules which has references to guides above are formulated briefly. From the other 
side different rules contains a reason and are formulated in details.

We use C++17 standard.

## Rules

- Use can use [Editor Guideline extension](https://marketplace.visualstudio.com/items?itemName=PaulHarrington.EditorGuidelines) for Microsoft Visual Studio. It helps to control code line lingth.


### Header files

1. [Self-contained header](https://google.github.io/styleguide/cppguide.html#Self_contained_Headers) shold conatin all that it need. *Diff*: header file has extension *.h
2. [Use headers guards](https://google.github.io/styleguide/cppguide.html#The__define_Guard). *Diff*: define is in format `<PATH>_<FILE>_H`.
3. [Include what you use](https://google.github.io/styleguide/cppguide.html#Include_What_You_Use), do not rely on transitive inclusions.
4. Use forward declarations to remove headers coflicts and include `windows.h`-like files.
5. Define [inline functions](https://google.github.io/styleguide/cppguide.html#Inline_Functions) only when they are fewer than 10 lines.
6. Include headers in the following order: C system headers, C++ standard library headers, other libraries' headers, your project's headers, related header. Separate them with a blank line. For example: 
```
#include <vector>
#include <unordered_map>

#include <QAbstractItemModel>

#include "datatypes.h"

#include "module.h"
```

### Scoping

1. Use namespaces to unite functions and small data structures by meaning. Don't use in headers.
You may not use a using-directive to make all names from a namespace available.
```
using namespace foo;  // allowed only locally in methods and fucntions or in small source files up to 50 lines.
namespace supercool = ::super::cool;  // allowed only locally as well
```
2. `typedef` is forbidden. Use `using`.
2. Use [internal linkage](https://google.github.io/styleguide/cppguide.html#Internal_Linkage) inly in source files.
3. Don't leave "free" [nonmember, staic member and global functions](https://google.github.io/styleguide/cppguide.html#Nonmember,_Static_Member,_and_Global_Functions).
4. [Place function's variable in a narrowest scope](https://google.github.io/styleguide/cppguide.html#Local_Variables).
5. Be careful with [static and global variables](https://google.github.io/styleguide/cppguide.html#Static_and_Global_Variables).

### Classes

1. Avoid virtual method calls and initialization that can fail in [constructors](https://google.github.io/styleguide/cppguide.html#Doing_Work_in_Constructors).
2. Use the [explicit keyword](https://google.github.io/styleguide/cppguide.html#Implicit_Conversions) for conversion operators and single-argument constructors.
3. Make clear which [copy and move](https://google.github.io/styleguide/cppguide.html#Copyable_Movable_Types) operations the class supports.
4. Make everything more than passive object carrying data [be a class](https://google.github.io/styleguide/cppguide.html#Structs_vs._Classes).
5. Prefer struct instead of [pair and tuple](https://google.github.io/styleguide/cppguide.html#Structs_vs._Tuples).
6. Make [inheritance](https://google.github.io/styleguide/cppguide.html#Inheritance) be public. (Private should be discussed)
7. [Overload operators](https://google.github.io/styleguide/cppguide.html#Operator_Overloading) judiciously.
8. [Make classes' data members private](https://google.github.io/styleguide/cppguide.html#Access_Control).
9. [Order declaration](https://google.github.io/styleguide/cppguide.html#Declaration_Order):
   1. public, protected, private (omit empty sections, separate with a blank line)
   2. types, constants, factory functions, constructors and assignment operators, destructor, all other methods, data members

### Functions 

1. See rules on functions [inputs and outputs](https://google.github.io/styleguide/cppguide.html#Inputs_and_Outputs).
2. Write [short functions](https://google.github.io/styleguide/cppguide.html#Write_Short_Functions) up to 60 lines.
3. [Overload a function](https://google.github.io/styleguide/cppguide.html#Function_Overloading) when there are no semantic differences between variants.
4. Use [default arguments](https://google.github.io/styleguide/cppguide.html#Default_Arguments) carefully.
4. Use the new [trailing-return-type](https://google.github.io/styleguide/cppguide.html#trailing_return) form only in cases where it's 
required (such as lambdas) or where, by putting the type after the function's parameter list
6. All parameters less that 16 bytes should be passed by value. All greater parameters should be passed by const reference.

### Smart pointers

1. Use [smart pointers](https://google.github.io/styleguide/cppguide.html#Ownership_and_Smart_Pointers) for dynamically allocated objects. Do not design your code to use shared ownership without a very good reason.
2. Avoid using of shared smart pointers everywhere. Think you cannot use reference or raw pointer on always existing object.
3. If you use weak_ptr fon't forget to check result of locking. Never use `ptr->lock()->doSomeJob()`.

### Other

1. Use [rvalue references](https://google.github.io/styleguide/cppguide.html#Rvalue_references) only in certain special cases.
2. Use reference with const `const &` as a function paramater. If you need to put an output parameter it's better to use raw pointer and check it in function.
```
void fillNumbers(std::vector<int> &numbers) // forbidden
...
void fillNumbers(std::vector<int> *numbers) { // allowed only for filler-functions
    if(!numbers)
        return;
...
}
```
3. [Friends classes](https://google.github.io/styleguide/cppguide.html#Friends) are allowed in rare cases.
3. [We do not use exceptions](https://google.github.io/styleguide/cppguide.html#Exceptions). Incapsulate third-party libraries when they use exceptions.
4. Avoid using run-time type information [(RTTI)](https://google.github.io/styleguide/cppguide.html#Run-Time_Type_Information__RTTI_). Rethink the system if RTTI is required.
5. Use [C++-style casts](https://google.github.io/styleguide/cppguide.html#Casting). Using const_cast indicates bad system.
6. Use QFile, QDebug, QInfo instead of std::streams.
7. Use the [prefix form (++i) unless you need postfix](https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement) semantics.
8. Use [const](https://google.github.io/styleguide/cppguide.html#Use_of_const) whenever it makes sense (function parameters, methods, and non-local variables).
9. Put *const* before typename and follow const-correctness.
10. Use [constexpr](https://google.github.io/styleguide/cppguide.html#Use_of_constexpr) to define true constants or to ensure constant initialization.
11. [Interger types](https://google.github.io/styleguide/cppguide.html#Integer_Types). We use `int` and `char` when want to use some number, when we need exact type size we use int32_t, uint32_t, ...
12. Avoid defining when possible [macros](https://google.github.io/styleguide/cppguide.html#Preprocessor_Macros), prefer inline functions, enums, and const variables.
13. Use [nullptr](https://google.github.io/styleguide/cppguide.html#0_and_nullptr/NULL) for pointers, and '\0' for chars (and not the 0 literal).
14. Prefer [sizeof(varname)](https://google.github.io/styleguide/cppguide.html#sizeof) to sizeof(type).
15. Use [type deduction](https://google.github.io/styleguide/cppguide.html#Type_deduction) only to make the code clearer or safer, and do not use it merely to avoid the inconvenience of writing an explicit type.
16. Use [class template argument deduction](https://google.github.io/styleguide/cppguide.html#CTAD) only with templates that have explicitly opted into supporting it.
17. Use [lambda expressions](https://google.github.io/styleguide/cppguide.html#Lambda_expressions) where appropriate. Prefer explicit captures when the lambda will escape the current scope.
19. Avoid complicated [template programming](https://google.github.io/styleguide/cppguide.html#Template_metaprogramming).
21. [Do not define specializations of std::hash](https://google.github.io/styleguide/cppguide.html#std_hash).
22. Nonstandard extensions to C++ may not be used unless otherwise specified.
23. Don't put an [alias](https://google.github.io/styleguide/cppguide.html#Aliases) in your public API just to save typing in the implementation; do so only if you intend it to be used by your clients.
24. Use `std::array` instead of plain arrays:
```
    int numbers[4];             // bad
    std::array<int, 4> numbers; // good
```
Plain arrays should be used only data which are filled with memory set functions (should be investigated).

25. Avoid return `std::pair` and `std::tuple` in internal code. They are forbidden in public interfaces. Use struct instead.
26. Wrap complex conditional expression into function or split it into several expressions.
27. Wrap long arithmetic expressions leaving arithmetic symbol on current line or split it into several expressions:
```
// bad
float a = b * c
    + d;
// good
float a = b * c +
          d;
```

28. Expression `auto value = _ui->someEdit()->text().toFloat()` with several `->`, `.` is ok until it doesn't require line wrapping. If wrapping is needed split expression into several parts.
29. Line wrapping inside loop definition is forbidden. Simplify expressions in loop definition.
20. Place `&` and `*` together with type.
28. Don't concatenate `QString().arg().arg()...` when arg is a string itself. Use `QString().arg(QString &, QString&, ...)` instead.
29. We name getters for custom widgets members as `Widget::checkBox()` without *get*. All other getters should start from *get*:`getWidth()`.

### early-return-rule

Return places should be minimized. 

```
void foo(){
    //only few rows of code (1-3, probably)
    if(!condition){
        return; // OK
  }
}

void foo(){
    // _many_ rows of code
    if(!condition){
        return; // Not OK. Difficult to find exit point
    }
}

void foo(){
    if(!condition){
        MegaClearingUpdatingFunction();
        ProbablyOneMoreMandatoryFoo(); // Code duplication.
        return; // Not OK. 
    }
    // Do work..
    MegaClearingUpdatingFunction(); // Code duplication. Easy to forget change "exit functions".
    ProbablyOneMoreMandatoryFoo();
}

// solution
void foo(){
    if(condition) {
        // Do work..
    }
    MegaClearingUpdatingFunction();
    ProbablyOneMoreMandatoryFoo();
}
```

### goto-rule
Whe you want to exit from two or more nested loops, use `goto`. It's the only case we approve `goto` usage. Below is not perfect code for which we suggest to use `goto`.

```
[&](){
    for( ... ; ... ; ... ) {
       for( ... ; ... ; ....) {
          if(contidion)
             return;
        }
    }
}();
//+1 nested construction, worse readability
```
A solution below is also bad, but popular:
```
for( ... ; ... && !doExit; ... ) {
       for( ... ; ... && !doExit ; ....) {
          if(contidion)
            doExit = true;
        }
    }
//worse perfomance, error prone (easy to forget && !doExit in some loop)
```
A perfect solution we prefer:
```
for( ... ; ... ; ... ) {
   for( ... ; ... ; ....) {
      if(contidion)
         goto m_loopEnd;
    }
}
m_loopEnd:
//perfect solution
```

## Inclusive language

1. Use [inclusive language](https://google.github.io/styleguide/cppguide.html#Inclusive_Language) in all code, including naming and comments.

## Naming

1. [Optimize for readability](https://google.github.io/styleguide/cppguide.html#General_Naming_Rules) using names that would be clear even to people on a different team. 
2. _Filenames_ should be all lowercase without any underscores `_` or dashes `-`. In general, make your filenames very specific. 
```
myclass.cpp // good
my_class.cpp // bad
myClass.cpp // bad
```
3. [Type names](https://google.github.io/styleguide/cppguide.html#Type_Names) start with a capital letter and have a capital letter for each new word, with no underscores: `MyExcitingClass`, `MyExcitingEnum`.
4. The names of common variables (including function parameters) should be in starting from `lowerCamelCase`. 
5. Class members should start with `_`
```
class Super {
    ...
    private:
        int _importantMember;
};
```
6. *Struct members* are in lowerCamelCase without `_`.
7. Variables declared `constexpr` or `const`, and whose value is fixed for the duration of the program, are named with a leading "c" followed by mixed case.
8. *Function names* are similar to class member functions: `lowerCamelCase`.
9. *Namespace names* are in `CamelCase`.
10. Enumerators (for both scoped and unscoped enums) should be named in `CamelCase`.
11. Give name to definitions like `UPPER_CASE_DEFITION_NAME` with words separated with `_`. Remember to avoid them.
12. Don't start each file with license boilerplate.
13. Use either the //  for comments.
14. Use the style below as a headers for classed,  functions, namespaces, etc. Don't duplicates comments in *.h and *.cpp.
```
/**
...
*/
```
13. File comments describe the contents of a file. If a file declares, implements, or tests exactly one abstraction that is documented by a comment at the point of declaration, file comments are not required. All other files must have file comments.
14. Comment any [complicated piece of code](https://google.github.io/styleguide/cppguide.html#Implementation_Comments), where self-commenting is not enough. Any tricky decision should be commented.
15. Use more [comments](https://google.github.io/styleguide/cppguide.html#Variable_Comments) for class members and global variables.
16. Comment nonobvious [functions arguments](https://google.github.io/styleguide/cppguide.html#Function_Argument_Comments).
17. Pay attention to [punctuation, spelling, and grammar](https://google.github.io/styleguide/cppguide.html#Punctuation,_Spelling_and_Grammar).
18. Use [TODO](https://google.github.io/styleguide/cppguide.html#TODO_Comments) comments for code that is temporary, a short-term solution, or good-enough but not perfect.



## Formatting

1. [Line length](https://google.github.io/styleguide/cppguide.html#Line_Length) should be up to 100 characters long.
2. [Non-ASCII characters](https://google.github.io/styleguide/cppguide.html#Non-ASCII_Characters) should be rare, and must use UTF-8 formatting.
3. Use only space, not tabs, and indent 4 spaces at a time.
4. [Return type](https://google.github.io/styleguide/cppguide.html#Function_Declarations_and_Definitions) on the same line as function name, parameters on the same line if they fit.
*Diff:* Use 8 spaces for wrapped functions parameters.
5. Use the following rules for [function declarations and definitions](https://google.github.io/styleguide/cppguide.html#Function_Declarations_and_Definitions).
*Diff*: Default indentation is 4 spaces. Wrapped parameters have a 8 space indent.
6. Format parameters and bodies for *lambda functions* as for any other function.
7. [Floating-point literals](https://google.github.io/styleguide/cppguide.html#Floating_Literals) should always have a radix point, with digits on both sides, even if they use exponential notation.
8. See rules for [functions calls](https://google.github.io/styleguide/cppguide.html#Function_Calls).
9. [Format a braced initializer](https://google.github.io/styleguide/cppguide.html#Braced_Initializer_List_Format) list exactly like you would format a function call in its place.
10. For conditionals put one space between the if and the opening parenthesis, and between the closing parenthesis and opening curly brace, but no spaces between the parentheses and the condition or initializer. If the optional initializer is present,
put a space or newline after the semicolon, but not before.
```
if (condition) {
    doOneThing();
} else {
    doAnotherThing();
}
```
Don't skip curly braces if *all* branches has only one statement. 
2. [Switch statements](https://google.github.io/styleguide/cppguide.html#Loops_and_Switch_Statements) may use braces for blocks. Loops shold use them always.
```
switch (var) {
    case 0: {  // 4 space indent
        ...    // 8 space indent
        break;
    }
    case 1: {
        ...
        break;
    }
    default: {
        assert(false);
    }
}
```
3. No spaces around period or arrow. [Pointer operators](https://google.github.io/styleguide/cppguide.html#Pointer_and_Reference_Expressions) do not have trailing spaces.
4. Break [boolean expressions](https://google.github.io/styleguide/cppguide.html#Boolean_Expressions) that is longer than the standard line length.
5. Do not needlessly surround the [return expression](https://google.github.io/styleguide/cppguide.html#Return_Values) with parentheses.
6. You may choose between `=`, `()`, and `{}` for [variable and array initialization](https://google.github.io/styleguide/cppguide.html#Variable_and_Array_Initialization).
7. The hash mark `#` that starts a *preprocessor directive* should always be at the beginning of the line. The space after hash and indents are not allowed.
```
  #ifdef STAR             // bad - indent
    doAnything();
# else                  // bad - space after #
    doAnother();
#endif // STAR          // good - mention directive on closing
```
8.Format clas as following:
```
class MyClass : public OtherClass    // Base class name on the same line as subclass anme
{
public:         // public, private, protected starts from ne wline without indents
    MyClass();  // Regular 4 space indent.
    explicit MyClass(int var);
    ~MyClass() {}

    void someFunction();
    void someFunctionThatDoesNothing() {}

    void setSomeVar(int var) { someVar = var; }
    int someVar() const { return someVar; }

private:       // Separate keyword with a blank line when it is not first 
    bool someInternalFunction();

    int _someVar;
    int _someOtherVar;
};
```
3. See rules for [contructor initializer lists](https://google.github.io/styleguide/cppguide.html#Constructor_Initializer_Lists).
4. The [contents of namespaces](https://google.github.io/styleguide/cppguide.html#Namespace_Formatting) are not indented.
5. Use of horizontal whitespace depends on location. Never put trailing whitespace at the end of a line.
- General
```
void f(bool b) {  // Open braces should be on the same line.
  ...
int i = 0;  // Semicolons have no space before them.
// Spaces inside braces for braced-init-list are not allowed.
int x[] = { 0 }; // bad
int x[] = {0};   // good

// Spaces around the colon in inheritance and initializer lists.
class Foo : public Bar {
public:
  // For inline function implementations, put spaces between the braces
  // and the implementation itself.
  Foo(int b) : Bar(), baz_(b) {}  // No spaces inside empty braces.
  void reset() { baz_ = 0; }  // Spaces separating braces from implementation.
  ...
```
- Loops and Conidtionals
```
if (b) {    // Space after the keyword in conditions and loops.
} else {    // Spaces around else.
}
while (test) {}   // There is usually no space inside parentheses.
switch (i) {
    for (int i = 0; i < 5; ++i) {
...
// Range-based for loops always have a space before and after the colon.
for (auto x : counts)
{
  ...
}
switch (i) {
  case 1:         // No space before colon in a switch case.
    ...
  case 2: break;  // Use a space after a colon if there's code after it.
```
- Operators
```
// Assignment operators always have spaces around them.
x = 0;

// Other binary operators usually have spaces around them. Parentheses should have no
// internal padding.
v = w * x + y / z; // good
v = w*x + y/z;     // bad
v = w * (x + z);   // good

// No spaces separating unary operators and their arguments.
x = -5;
++x;
if (x && !y)
  ...
```
- Templates and casts
```
// No spaces inside the angle brackets (< and >), before
// <, or between >( in a cast
std::vector<std::string> x;
// No spaces between type and pointer are OK, but be consistent.
y = static_cast<char*>(x);
```
3. Minimize [vertical spaces](https://google.github.io/styleguide/cppguide.html#Vertical_Whitespace). Don't use more than *one blank line* between functions. Resist starting function from blank line.


## Logging levels are Qt native

1.  We have 5 commonly used logging levels:
    1. qDebug() is used for writing custom debug output (some useful info)
    2. qInfo() is used for informational messages (e.g. some important events: experiment loading, ROI category creation or switching, etc.)
    3. qWarning() is used to report warnings and recoverable errors in your application (e.g. material is not linked, ROI creation failed, etc.)
    4. qCritical() is used for writing critical error messages and reporting system errors (e.g. memory cannot be allocated, etc.)
    5. qFatal() is used for writing fatal error messages shortly before exiting

## Qt using rules

1. Don't inherit UI Qt classes (from *.ui file) have them as a pointer member.
2. Put signals/slots blocks in corresponding public/protected/private blocks are methods befor members.
3. Using connect without context object is forbidden.

