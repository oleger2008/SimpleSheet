#include <cassert>
#include <iostream>

  #include "common.h"
  #include "test_runner_p.h"

  inline std::ostream& operator<<(std::ostream& output, Position pos) {
      return output << "(" << pos.row << ", " << pos.col << ")";
  }

  inline Position operator"" _pos(const char* str, std::size_t) {
      return Position::FromString(str);
  }

  inline std::ostream& operator<<(std::ostream& output, Size size) {
      return output << "(" << size.rows << ", " << size.cols << ")";
  }

  inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
      std::visit(
          [&](const auto& x) {
              output << x;
          },
          value);
      return output;
  }

  namespace {

  void TestEmpty() {
      auto sheet = CreateSheet();
      ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{0, 0}));
  }

  void TestInvalidPosition() {
      auto sheet = CreateSheet();
      try {
          sheet->SetCell(Position{-1, 0}, "");
      } catch (const InvalidPositionException&) {
      }
      try {
          sheet->GetCell(Position{0, -2});
      } catch (const InvalidPositionException&) {
      }
      try {
          sheet->ClearCell(Position{Position::MAX_ROWS, 0});
      } catch (const InvalidPositionException&) {
      }
  }

  void TestSetCellPlainText() {
      auto sheet = CreateSheet();

      auto checkCell = [&](Position pos, std::string text) {
          sheet->SetCell(pos, text);
          CellInterface* cell = sheet->GetCell(pos);
          ASSERT(cell != nullptr);
          ASSERT_EQUAL(cell->GetText(), text);
          ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text);
      };

      checkCell("A1"_pos, "Hello");
      checkCell("A1"_pos, "World");
      checkCell("B2"_pos, "Purr");
      checkCell("A3"_pos, "Meow");

      const SheetInterface& constSheet = *sheet;
      ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr");

      sheet->SetCell("A3"_pos, "'=escaped");
      CellInterface* cell = sheet->GetCell("A3"_pos);
      ASSERT_EQUAL(cell->GetText(), "'=escaped");
      ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped");
  }

  void TestClearCell() {
      auto sheet = CreateSheet();

      sheet->SetCell("C2"_pos, "Me gusta");
      sheet->ClearCell("C2"_pos);
      ASSERT(sheet->GetCell("C2"_pos) == nullptr);

      sheet->ClearCell("A1"_pos);
      sheet->ClearCell("J10"_pos);
  }
  void TestPrint() {
      auto sheet = CreateSheet();
      sheet->SetCell("A2"_pos, "meow");
      sheet->SetCell("B2"_pos, "=1+2");
      sheet->SetCell("A1"_pos, "=1/0");

      ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 2}));

      std::ostringstream texts;
      sheet->PrintTexts(texts);
      ASSERT_EQUAL(texts.str(), "=1/0\t\nmeow\t=1+2\n");

      std::ostringstream values;
      sheet->PrintValues(values);
      ASSERT_EQUAL(values.str(), "#DIV/0!\t\nmeow\t3\n");

      sheet->ClearCell("B2"_pos);
      ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 1}));
  }
  void TestExample() {
      auto sheet = CreateSheet();
      sheet->SetCell("A1"_pos, "=(1+2)*3");
      sheet->SetCell("B1"_pos, "=1+(2*3)");

      sheet->SetCell("A2"_pos, "some");
      sheet->SetCell("B2"_pos, "text");
      sheet->SetCell("C2"_pos, "here");

      sheet->SetCell("C3"_pos, "\'and");
      sheet->SetCell("D3"_pos, "\'here");

      sheet->SetCell("B5"_pos, "=1/0");

      std::ostringstream values;
      sheet->PrintValues(values);
      ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{5, 4}));
      ASSERT_EQUAL(values.str(), "9\t7\t\t\nsome\ttext\there\t\n\t\tand\there\n\t\t\t\n\t#DIV/0!\t\t\n");

      std::ostringstream texts;
      sheet->PrintTexts(texts);
      ASSERT_EQUAL(texts.str(), "=(1+2)*3\t=1+2*3\t\t\nsome\ttext\there\t\n\t\t\'and\t\'here\n\t\t\t\n\t=1/0\t\t\n");

      sheet->ClearCell("B5"_pos);
      ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{3, 4}));
  }

  void TestCorrectFormula() {
      using namespace std;
      auto sheet = CreateSheet();
      try {
          sheet->SetCell(Position{0, 0}, "=A1+*");
      } catch (const FormulaException&) {
      } catch (...) {
          assert(false);
      }
  }

  }  // namespace

  int main() {
      TestRunner tr;
      RUN_TEST(tr, TestEmpty);
      RUN_TEST(tr, TestInvalidPosition);
      RUN_TEST(tr, TestSetCellPlainText);
      RUN_TEST(tr, TestClearCell);
      RUN_TEST(tr, TestPrint);

      RUN_TEST(tr, TestExample);
      RUN_TEST(tr, TestCorrectFormula);
      return 0;
  }
  
