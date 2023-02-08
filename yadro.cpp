#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <istream>
#include <variant>
#include <exception>
#include <iomanip>
#include <algorithm>
#include <iostream>


constexpr double eps = 1e-15;


class Table {

  struct Cell {
    int column;
    int row_name;
  };

  using Token = std::variant<Cell, double, char>;
  using CellData = std::variant<double, std::vector<Token>>;

  const std::vector<char> operations = { '+', '-', '*', '/' };
  std::vector<std::vector<CellData>> table_;
  std::vector<std::string> columns_;
  std::vector<std::string> rows_;
  std::unordered_map<std::string, int> columns_map_;
  std::unordered_map<int, int> rows_map_;

  std::vector<std::vector<bool>> used;

public:
  Table(const std::string& filename) {
    std::ifstream test_file;
    test_file.open(filename);
    if (!test_file) {
      std::cout << "Cannot open file" << std::endl;
      std::exit(1);
    }
   
    std::string line;
    std::getline(test_file, line);
    DeleteSpaces(line);
    std::istringstream s(line);

    std::string token;
    std::getline(s, token, ',');
    int i = 0;
    while (std::getline(s, token, ','))
    {
      columns_.push_back(token);
      columns_map_[token] = i++;
    }

    int j = 0;
    try {
      while (std::getline(test_file, line)) {
        DeleteSpaces(line);
        auto tokens = ParseLine(line);
        rows_.push_back(tokens.at(0));
        table_.push_back(std::vector<CellData>(i));
        if (tokens.size() != columns_.size() + 1) {
          throw std::runtime_error("Invalid table");
        }
        for (size_t idx = 1; idx < tokens.size(); ++idx) {
          table_[j][idx - 1] = ParseFormula(tokens.at(idx));
        }
        rows_map_[ParseNumber(tokens.at(0))] = j++;
      }
    }
    catch (std::exception& ex) {
      std::cout << ex.what() << std::endl;
      std::exit(1);
    }
    catch (...) {
      std::cout << "Invalid table" << std::endl;
      std::exit(1);
    }

    used = std::vector<std::vector<bool>>(j, std::vector<bool>(i, false));

  }


  void DeleteSpaces(std::string& line) {
    line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char a){
      return std::isspace(a);
    }), line.end());
  }

  CellData ParseFormula(const std::string& formula) {
    if (formula[0] != '=') {
      return static_cast<double>(ParseNumber(formula));
    }

    std::vector<Token> answer(3);
    size_t idx = 0;
    while (idx < operations.size() && formula.find(operations[idx]) == std::string::npos) {
      ++idx;
    }
    if (idx == operations.size()) {
      throw std::runtime_error("No operator in formula");
    }
    char operation = operations[idx];
    size_t pos = formula.find(operation);

    std::string first_cell = formula.substr(1, pos - 1);
    std::string second_cell = formula.substr(pos + 1);

    answer[0] = ParseCell(first_cell);
    answer[1] = operation;
    answer[2] = ParseCell(second_cell);

    return answer;
  }


  Token ParseCell(const std::string& cell) {
    size_t i = 0;
    while (i < cell.size() && !std::isdigit(cell[i])) {
      ++i;
    }
    if (i == cell.size()) {
      throw std::runtime_error("No row in cell");
    }
    else if (i == 0) {
      return static_cast<double>(ParseNumber(cell));
    }
    Cell ans = { .column = columns_map_.at(cell.substr(0, i)), .row_name = ParseNumber(cell.substr(i))  };
    return ans;
  }

  int ParseNumber(const std::string& cell_number) {
    for (const char& ch: cell_number) {
      if (!std::isdigit(ch)) {
        throw std::runtime_error("Incorrect data in cell");
      }
    }
    try {
      return std::stoi(cell_number);
    }
    catch (...) {
      throw std::runtime_error("Incorrect data in cell");
    }
  }

  std::vector<std::string> ParseLine(const std::string& line) {
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> res;
    while (std::getline(ss, token, ','))
    {
      res.push_back(token);
    }
    return res;
  }


  void Calculate() {
    for (size_t row = 0; row < rows_.size(); ++row) {
      for (size_t col = 0; col < columns_.size(); ++col) {
        try {
          Calculate(row, col);
        } 
        catch (std::out_of_range& ex) {
          std::cout << "Invalid cell references in formula" << std::endl;
          std::exit(1);
        }
        catch (std::exception& ex) { // 
          std::cout << ex.what() << std::endl;
          std::exit(1);
        }
      }
    }
  }


  double Calculate(int row, int col) {
    if (std::holds_alternative<double>(table_[row][col])) {
      return std::get<double>(table_[row][col]);
    }

    if (used[row][col]) {
      throw std::runtime_error("Infinity loop");
    }
    used[row][col] = true;
    auto expression = std::get<std::vector<Token>>(table_[row][col]);
    double lvalue = 0, rvalue = 0;
    if (std::holds_alternative<double>(expression[0])) {
      lvalue = std::get<double>(expression[0]);
    }
    else {
      auto cell = std::get<Cell>(expression[0]);
      lvalue = Calculate(rows_map_.at(cell.row_name), cell.column);
    }

    if (std::holds_alternative<double>(expression[2])) {
      rvalue = std::get<double>(expression[2]);
    }
    else {
      auto cell = std::get<Cell>(expression[2]);
      rvalue = Calculate(rows_map_.at(cell.row_name), cell.column);
    }
    
    char operation = std::get<char>(expression[1]);
    switch (operation)
    {
    case '+':
    {
      table_[row][col] = lvalue + rvalue;
      break;
    }
    case '-':
    {
      table_[row][col] = lvalue - rvalue;
      break;
    }
    case '*':
    {
      table_[row][col] = lvalue * rvalue;
      break;
    }
    case '/':
    {
      if (rvalue < eps) {
        throw std::runtime_error("Division by zero");
      }
      table_[row][col] = lvalue / rvalue;
      break;
    }
    }
    return std::get<double>(table_[row][col]);
  }


  void Print() {
    std::cout << ", ";
    for (size_t jdx = 0; jdx + 1 < columns_.size(); ++jdx) {
      std::cout << columns_[jdx] << ", ";
    }
    std::cout << columns_.back();
    std::cout << std::endl;
    for (size_t idx = 0; idx < rows_.size(); ++idx) {
      std::cout << rows_[idx] << ", ";
      for (size_t jdx = 0; jdx + 1 < columns_.size(); ++jdx) {
        std::cout << std::get<double>(table_[idx][jdx]) << ", ";
      }
      std::cout << std::get<double>(table_[idx].back());
      std::cout << std::endl;
    }
  }
};


int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Too few arguments" << std::endl;
    return 0;
  }

  char* filename = argv[1];

  Table table(filename);
  table.Calculate();
  table.Print();
}


