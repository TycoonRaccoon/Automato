#include <unordered_map>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <stack>

class Automaton {
private:
  enum State {
    A, B, C, D, E, F, G, H, I, J, K
  };

  State _current_state = State::A;
  std::unordered_map<std::string, float>& _variables;
  std::string _identifier;
  std::string _aux;
  std::stack<float> _values;
  std::stack<char> _ops;

  int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
  }

  float applyOp(float a, float b, char op) {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/': return a / b;
    default: throw std::invalid_argument("Invalid operator");
    }
  }

  void proccessLastOp() {
    float b = _values.top(); _values.pop();
    float a = _values.top(); _values.pop();
    char op = _ops.top(); _ops.pop();
    _values.push(applyOp(a, b, op));
  }

  void proccessOps(char ch) {
    while (!_ops.empty() && precedence(_ops.top()) >= precedence(ch))
      proccessLastOp();
    _ops.push(ch);
  }

  void proccessFloatValue() {
    _values.push(std::stof(_aux));
    _aux = "";
  }

  void proccessVariableValue() {
    if (_variables.find(_aux) == _variables.end()) throw std::invalid_argument(std::string("Invalid variable '") + _aux + "'");
    _values.push(_variables[_aux]);
    _aux = "";
  }

  float calculate() {
    while (!_ops.empty())
      proccessLastOp();

    return _values.top();
  }

  void reset() {
    _current_state = State::A;
    _identifier = "";
    _aux = "";
    while (!_values.empty())
      _values.pop();
    while (!_ops.empty())
      _ops.pop();
  }

  State transitionsA(char ch) {
    if (ch == ' ') return State::A;
    if (islower(ch)) {
      _identifier += ch;
      return State::B;
    }
    throw std::invalid_argument("First char at identifier must be a lowercase letter");
  };

  State transitionsB(char ch) {
    if (islower(ch) || ch == '_' || isdigit(ch)) {
      _identifier += ch;
      return State::B;
    }
    if (ch == ' ')
      return State::C;
    if (ch == '=')
      return State::D;
    throw std::invalid_argument(std::string("Invalid char '") + ch + "' at identifier");
  };

  State transitionsC(char ch) {
    if (ch == ' ')
      return State::C;
    if (ch == '=')
      return State::D;
    throw std::invalid_argument("Expecting '='");
  };

  State transitionsD(char ch) {
    if (ch == ' ')
      return State::D;
    if (islower(ch)) {
      _aux += ch;
      return State::E;
    }
    if (isdigit(ch)) {
      _aux += ch;
      return State::F;
    }
    throw std::invalid_argument("Expecting a variable or number");
  };

  State transitionsE(char ch) {
    if (islower(ch) || ch == '_' || isdigit(ch)) {
      _aux += ch;
      return State::E;
    }
    if (ch == '*' || ch == '+' || ch == '-' || ch == '/') {
      proccessVariableValue();
      proccessOps(ch);
      return State::D;
    }
    if (ch == ' ') return State::J;
    if (ch == ';') {
      proccessVariableValue();
      return State::I;
    }
    throw std::invalid_argument(std::string("Invalid char '") + ch + "' at identifier");
  };

  State transitionsF(char ch) {
    if (isdigit(ch)) {
      _aux += ch;
      return State::F;
    }
    if (ch == '*' || ch == '+' || ch == '-' || ch == '/') {
      proccessFloatValue();
      proccessOps(ch);
      return State::D;
    }
    if (ch == ' ') return State::K;
    if (ch == '.') {
      _aux += ch;
      return State::G;
    }
    if (ch == ';') {
      proccessFloatValue();
      return State::I;
    }
    throw std::invalid_argument(std::string("Invalid char '") + ch + "' at value");
  };

  State transitionsG(char ch) {
    if (isdigit(ch)) {
      _aux += ch;
      return State::H;
    }
    throw std::invalid_argument("Float value must have at least one decimal precision");
  };

  State transitionsH(char ch) {
    if (isdigit(ch)) {
      _aux += ch;
      return State::H;
    }
    if (ch == '*' || ch == '+' || ch == '-' || ch == '/') {
      proccessFloatValue();
      proccessOps(ch);
      return State::D;
    }
    if (ch == ';') {
      proccessFloatValue();
      return State::I;
    }
    throw std::invalid_argument(std::string("Invalid char '") + ch + "' at value");
  };

  State transitionsI(char ch) {
    throw std::invalid_argument("Can not have chars after ';'");
  };

  State transitionsJ(char ch) {
    if (ch == ' ') {
      return State::J;
    }
    if (ch == '*' || ch == '+' || ch == '-' || ch == '/') {
      proccessVariableValue();
      proccessOps(ch);
      return State::D;
    }
    throw std::invalid_argument("Expecting an operation");
  };

  State transitionsK(char ch) {
    if (ch == ' ') {
      return State::K;
    }
    if (ch == '*' || ch == '+' || ch == '-' || ch == '/') {
      proccessFloatValue();
      proccessOps(ch);
      return State::D;
    }
    throw std::invalid_argument("Expecting an operation");
  };

  std::unordered_map<State, std::function<State(char)>> states = {
    {State::A, [this](char ch) { return transitionsA(ch); }},
    {State::B, [this](char ch) { return transitionsB(ch); }},
    {State::C, [this](char ch) { return transitionsC(ch); }},
    {State::D, [this](char ch) { return transitionsD(ch); }},
    {State::E, [this](char ch) { return transitionsE(ch); }},
    {State::F, [this](char ch) { return transitionsF(ch); }},
    {State::G, [this](char ch) { return transitionsG(ch); }},
    {State::H, [this](char ch) { return transitionsH(ch); }},
    {State::I, [this](char ch) { return transitionsI(ch); }},
    {State::J, [this](char ch) { return transitionsJ(ch); }},
    {State::K, [this](char ch) { return transitionsK(ch); }}
  };

public:
  Automaton(std::unordered_map<std::string, float>& variables) : _variables(variables) {}

  float processInput(const std::string& input) {
    for (char c : input)
      _current_state = states[_current_state](c);
    if (_current_state != State::I) throw std::invalid_argument("Missing ';'");

    float value = calculate();
    _variables[_identifier] = value;

    reset();
    return value;
  }
};

void parseDefinedVariables(std::ifstream& file_stream, std::unordered_map<std::string, float>& variables) {
  std::string str;

  while (std::getline(file_stream, str)) {
    if (str.empty()) break;
    std::string delimiter = " = ";
    std::string name = str.substr(0, str.find(delimiter));
    std::string value = str.substr(str.find(delimiter) + delimiter.size(), str.size());
    variables[name] = std::stof(value);
  }
}

void proccessAttrib(std::ifstream& file_stream, std::unordered_map<std::string, float>& variables) {
  std::string str;
  Automaton automaton(variables);

  try {
    while (std::getline(file_stream, str))
      std::cout << "attrib '" << str << "'\nresult: " << automaton.processInput(str) << std::endl;
  } catch (const std::invalid_argument& e) {
    std::cerr << str << ": " << e.what() << '\n';
  }
}

void printVariables(std::unordered_map<std::string, float>& variables) {
  std::cout << std::endl << "variables:" << std::endl;
  for (auto const& [key, val] : variables)
    std::cout << key << ": " << val << std::endl;
}

int main() {
  std::unordered_map<std::string, float> variables;

  std::ifstream file_stream("input.txt");

  parseDefinedVariables(file_stream, variables);
  proccessAttrib(file_stream, variables);

  file_stream.close();

  printVariables(variables);

  return 0;
}
