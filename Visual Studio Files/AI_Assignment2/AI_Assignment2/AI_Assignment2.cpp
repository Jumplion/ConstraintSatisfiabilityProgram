/*
In this problem, you will implement a CSP solver that takes exactly three arguments from the command line:

1.  A .var file that contains the variables in the CSP to be solved and their domains. 
    Each line of the file contains a variable (represented by a single letter), followed by 
    a colon and its possible values, each of which is an integer. For instance, the line “A: 1 2 3” 
    indicates that the possible values for variable A are 1, 2, and 3. Note that there is a space 
    separating the domain values.

2.  A .con file that contains the constraints. Each line corresponds to exactly one constraint, 
    which involves two variables and has the form VAR1 OP VAR2. VAR1 and VAR2 are the names of 
    the two variables involved, and OP can be one of four binary operators: = (equality),
    ! (inequality), > (greater than), and < (less than).

3.  The consistency-enforcing procedure, which can take one of two values: none and fc. 
    If none is used, no consistency-enforcing procedure is applied, and the solver simply uses 
    backtracking to solve the problem. fc indicates that the solver will use forward checking to
    enforce consistency. 
    Note:

    Most ConstrainED
      - Variable with the FEWEST values available
      - Minimum Remaining Values

    Most ContrainING
      - Variable that imposes the MOST constraints on the remaining variables (has the most constraints)

      *   Whenever the solver needs to choose a VARIABLE during the search process, apply the most
          constrainED variable heuristic, breaking ties using the most constrainING variable heuristic.
          If more than one variable remains after applying these heuristics, break ties alphabetically.

      •   Whenever the solver needs to choose a VALUE during the search process, apply the least
          constraining value heuristic. If more than one value remains after applying this heuristic,
          break ties by preferring smaller values.

Your program should allow exactly three arguments to be specified in the command line invocation
of your program, in the order in which they are listed above. Sample input files will be
available in the assignment page of the course website. Your program should write to stdout
the first 30 branches visited in the search tree (or stop when the solution is reached). Write them
in text form with each branch on one line. For example, suppose we had variables X and Y with
domains {0, 1, 2}, variable Z with domain {1, 2}, and constraint Y=Z. 

The branches visited in the search tree without forward checking should be written as:

1. Z=1, X=0, Y=0 failure
2. Z=1, X=0, Y=1 solution

However, if forward checking is applied, the output should be:

1. Z=1, Y=1, X=0 solution

Do not output anything else to stdout. Any program that does not conform to the above specification
will receive no credit. To implement the program, you may use C, C++, Java, or any
programming languages pre-approved by your dear TA, Nathan Hayes, for the previous assignments.
If you have any questions, post them on piazza.

Submit via eLearning (i) your source code and (ii) a README file that contains instructions
for compiling and running your program (as well as the platform (Windows/Linux/Solaris) on
which you developed your program). Each group should turn in a single copy of the code and the
names of all group members should appear in the README file.
*/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>

using namespace std;

struct Variable;
struct Constraint;

struct Variable 
{
  public:
    char name;
    int value;
    bool assigned = false;
    vector<int> domain;
    queue<int> remainingValues;
    vector<Constraint*> constraints;

    Variable(char varName, vector<int> values) 
    {
      name = varName;
      domain = values;
      // Initialized value
      value = domain[0];
      ResetQueue();
    }

    void ResetQueue() 
    {
      for (int x = 0; x < domain.size(); x++)
        remainingValues.push(domain[x]);
    }

    void Print() 
    {
      cout << name << ": ";
      for (int x = 0; x < domain.size(); x++)
        cout << domain[x] << " ";
      cout << endl;
    }
};

struct Constraint 
{
  public:
    Variable* mainVar;
    Variable* compareVar;

    char symbol;
    int type = -1;

    Constraint(Variable* main, Variable* compare, char conSymbol) 
    {
      mainVar = main;
      compareVar = compare;

      symbol = conSymbol;

      if (conSymbol == '=')
        type = 0;
      else if (conSymbol == '!')
        type = 1;
      else if (conSymbol == '>')
        type = 2;
      else if (conSymbol == '<')
        type = 3;
      else
        type = 0;
    }

    bool IsSatisfied() 
    {
      switch (type)
      {
        case 0: return Equals(); break;
        case 1: return NotEquals(); break;
        case 2: return GreaterThan(); break;
        case 3: return LessThan(); break;
      }
    }

  private:
    bool Equals() 
    {
      return mainVar->value == compareVar->value;
    }
    bool NotEquals() 
    {
      return mainVar->value != compareVar->value;
    }
    bool GreaterThan() 
    {
      return mainVar->value > compareVar->value;
    }
    bool LessThan() 
    {
      return mainVar->value < compareVar->value;
    }
};

#pragma region
Variable* FindVariable(char);
vector<Variable*> BackCheck(vector<Variable*>);
bool CheckIfValid(vector<Variable*>);
void SortVariables();
void SortValues(Variable*);

vector<Variable*> variables;
vector<Constraint*> constraints;
stack<Variable*> remainingVariables;
ifstream varFile;
ifstream conFile; 

int steps = 0;
bool solutionFound = false;
#pragma endregion

int main(int argc, char *argv[])
{
#pragma region
  bool forwardCheck = false;

  if (argc > 3)
    forwardCheck = true;

  varFile.open(argv[1]);
  conFile.open(argv[2]);

  string varLine = "";
  // var file input example
  // A: 1 2 3 4 5
  // B: 1 2 3 4 5
  // C: 1 2 3 4 5
  // D: 1 2 3 4 5
  // E: 1 2 3
  // F: 1 2
  while (getline(varFile, varLine))
  {
    if (varLine.size() > 0 && varLine[0] != ' ' && varLine[0] != NULL)
    {
      vector<int> values;

      for(int x=3; x<varLine.length(); x += 2)
        values.push_back((int)varLine[x] - 48);
      
      Variable* v = new Variable(varLine[0], values);
      variables.push_back(v);
    }
  }

  string conLine = "";
  // constraint file input example
  // A > B
  // B > F
  // A > C
  // C > E
  // A > D
  // D = E
  while (getline(conFile, conLine))
  {
    if (conLine.size() > 0 && conLine[0] != ' ' && conLine[0] != 0)
    {
      Variable* main = FindVariable(conLine[0]);
      Variable* compare = FindVariable(conLine[4]);
      
      main->constraints.push_back(new Constraint(main, compare, conLine[2]));

      constraints.push_back(new Constraint(main, compare, conLine[2]));
    }
  }

  varFile.close();
  conFile.close();

#pragma endregion

  SortVariables();
  for (int x = 0; x < variables.size(); x++)
    remainingVariables.push(variables[x]);

  vector<Variable*> startingVector;
  std::cout << endl << "Steps:" << endl;

  //if (forwardCheck)
  //ForwardCheck(startingVector);
  //else
  BackCheck(startingVector);

  return 0;
}

Variable* FindVariable(char name) 
{
  for (int x = 0; x < variables.size(); x++)
    if (variables[x]->name == name)
      return variables[x];
}

// Typical solution
// 1. F = 1, E = 1, A = 5, B = 1  failure
// 2. F = 1, E = 1, A = 5, B = 2, C = 1  failure
// 3. F = 1, E = 1, A = 5, B = 2, C = 2, D = 1  solution
vector<Variable*> BackCheck(vector<Variable*> vars)
{
  // Check if a solution has already been found
  // If it's too big, return it
  // Check if it's a solution after all
  // If it fails all of those, are we out of steps anyway?
  if (solutionFound || vars.size() > variables.size() || CheckIfValid(vars) || remainingVariables.empty() || steps >= 30) 
  { return vars; }

  // Check if there are enough remaining variables to check for anyway
  else
  {
    // Get the top variable of the remainingVariables stack and pop it
    Variable* v = remainingVariables.top();
    remainingVariables.pop();

    // Sort Remaining VALUES of the variable.
    SortValues(v);

    // Go through the possible values of the current variable
    // If the current variable has no more values to check, return the original vars list.
    while (v->remainingValues.size() > 0)
    {
      // Set the variable's current value to the next remainingValue in the queue
      v->value = v->remainingValues.front();
      v->remainingValues.pop();

      bool ok = true;

      // Go through this variable's constraints
      // Check if it fails any of them
      for (int y = 0; y < v->constraints.size(); y++)
      {
        // Compare only to the variables that have been assigned a value
        // If it fails at any point, break the loop
        if (v->constraints[y]->compareVar->assigned && !v->constraints[y]->IsSatisfied())
        {
          ok = false;
          break;
        }
      }

      // If the variable at the current variable passes its constraints, push it to a new list.
      if (ok)
      {
        v->assigned = true;

        // Copy the vars list to a newVars list
        vector<Variable*> newVars = vars;
        newVars.push_back(v);

        // Just in case, if newVars has more variables than it should, return the vars
        if (newVars.size() > variables.size()) { return vars; }

        // Otherwise, if it's the same size, check if it's a solution
        // If it's not a solution, continue the search;
        if (newVars.size() == vars.size() && !solutionFound && CheckIfValid(newVars)) { return newVars; }

        // If we still have more variables to check, BackCheck the newVars list
        // If the list that is returned is the same size as variables
        else
          BackCheck(newVars);

        v->assigned = false;

        // If newVars and variables have the same number of variables in them, check if they're valid
        if (newVars.size() == variables.size() && CheckIfValid(newVars)) { return newVars; }
      }

      // If it fails, but we already found a solution, don't bother printing the failure
      else if (!solutionFound)
      {
        steps++;

        cout << steps << ": ";
        for (int x = 0; x < vars.size(); x++)
          std::cout << vars[x]->name << " = " << vars[x]->value << ", ";
        std::cout << v->name << " = " << v->value << " Failure" << endl;
      }

      if (steps >= 30) { return vars; }
    }

    // If we went through each of the remaining values for the variable
    // and we havn't found a solution, return vars so the previous
    // stack in the recursion can go through its values

    v->assigned = false;
    // Reset the values for this variable
    v->ResetQueue();
    remainingVariables.push(v);
    return vars;
  }
}

bool CheckIfValid(vector<Variable*> vars) 
{
  
  if (vars.size() == variables.size())
  {
    for (int x = 0; x < vars.size(); x++)
      for (int y = 0; y < vars[x]->constraints.size(); y++)
        if (!vars[x]->constraints[y]->IsSatisfied())
          return false;

    if (!solutionFound) 
    {
      steps++;
      cout << steps << ": ";
      for (int x = 0; x < vars.size(); x++)
        std::cout << vars[x]->name << " = " << vars[x]->value << ", ";
      std::cout << " Solution" << endl;
    }
    return solutionFound = true;
  }
  else
    return false;
}

#pragma region

/*
Most ConstrainED
- Variable with the FEWEST values available
- Minimum Remaining Values

Most ContrainING
- Variable that imposes the MOST constraints on the remaining variables(has the most constraints)

 Sort Variables by the MINIMUM REMAINING VALUES

  Whenever the solver needs to choose a VARIABLE during the search process, apply the most
  constrainED variable heuristic, breaking ties using the most constrainING variable heuristic.
  If more than one variable remains after applying these heuristics, break ties alphabetically.
*/
void SortVariables()
{
  vector<Variable*> vars;
  for (int x = 0; x < remainingVariables.size(); x++)
  {
    vars.push_back(remainingVariables.top());
    remainingVariables.pop();
  }

  for (int i = 1; i < vars.size(); ++i)
  {
    bool inplace = true;
    int j = 0;
    for (; j < i; ++j)
    {
      // Sort by Most ContrainED (fewest remaining values)
      if (vars[i]->remainingValues.size() > vars[j]->remainingValues.size())
        inplace = false;

      else if (vars[i]->remainingValues.size() == vars[j]->remainingValues.size())
      {
        // If it's a tie, sort by Most ContstraING (most constraints)
        if (vars[i]->constraints.size() < vars[j]->constraints.size())
          inplace = false;
        // Otherwise, sort alphabetically
        else if (vars[i]->name > vars[j]->name)
          inplace = false;
      }

      if (!inplace)
        break;
    }

    if (!inplace)
    {
      Variable* save = vars[i];
      for (int k = i; k > j; --k)
        vars[k] = vars[k - 1];

      vars[j] = save;
    }
  }

  for (int y = 0; y < vars.size(); y++) 
  {
    remainingVariables.push(vars.back());
    vars.pop_back();
  }

}


/*
Whenever the solver needs to choose a VALUE during the search process, apply the LEAST
constraining value heuristic. If more than one value remains after applying this heuristic,
break ties by preferring smaller values.
*/
void SortValues(Variable* var)
{
  for (int i = 1; i < var->domain.size(); ++i)
  {
    var->value = var->domain[i];

    bool inplace = true;
    int iSuccessCount = 0;
    int j = 0;

    // First check how many constraints the current I value succeeds in
    for (int x = 0; x < constraints.size(); x++)
      if (constraints[x]->IsSatisfied())
        iSuccessCount++;

    for (; j < i; ++j)
    {
      var->value = var->domain[j];

      int jSuccessCount = 0;
      // First check how many constraints the current J value succeeds in
      for (int y = 0; y < constraints.size(); y++)
        if (constraints[y]->IsSatisfied())
          jSuccessCount++;

      // Sort by Most ContrainED (fewest remaining values)
      if (iSuccessCount > jSuccessCount)
        inplace = false;
      else if (var->domain[i] > var->domain[j])
        inplace = false;

      if (!inplace)
        break;
    }

    // If we found a value out of place, sort it back in
    if (!inplace)
    {
      int save = var->domain[i];
      for (int k = i; k > j; --k)
        var->domain[k] = var->domain[k - 1];

      var->domain[j] = save;
    }
  }
}

#pragma endregion