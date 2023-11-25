#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <doctest.h>

#include "Profiler.h"

#include "Table.h"
#include "FeedbackTable.h"
#include "Variable.h"
#include "TestVariable.h"
#include "Helper.h"

TEST_SUITE("Table") {
  TEST_CASE("get() on one dimensional table") {
    double expected = 1234.0;

    Variable* dummy_source = new Variable(expected);
    v_double* dummy_anchors = new v_double();
    dummy_anchors->push_back(1);

    v_double* table_data = new v_double();
    table_data->push_back(expected);
    std::string table_name = "test_table_1d";

    Dimension* dimension_x = new Dimension(dummy_source, &LINEAR_INTEGRATION, dummy_anchors);
    v_dimension *dimensions = new std::vector<Dimension*>();
    dimensions->push_back(dimension_x);


    Table* table = new Table(&table_name, dimensions, table_data);
    double result = table->get();
    CHECK(result == expected);
  }

  TEST_CASE("get() on clamped dimensional table") {
    double expected = 0.0;

    Variable* throttle = new Variable(0.0);

    Table* throttle_pos = newTable(new std::string("throttle_pos"),
      newDimensionVec(
        newDimension(throttle, &LINEAR_INTEGRATION, { 
          0.10, 4.75
        })
      ), newDataVec({
        0.0, 1.0
      }));
    
      double result = throttle_pos->get();
      CHECK(result == expected);
  }

  TEST_CASE("get() on subtraction table") {
    double expected = 0.0;
    double temp_limit = 1000.0;

    TestVariable* temp_manifold_c = new TestVariable();
    TestVariable* temp_inlet_c = new TestVariable();

    Table* temp_delta = newTable(new std::string("temp_delta"),
      newDimensionVec(
        newDimension(temp_manifold_c, &LINEAR_INTEGRATION, { 
          -temp_limit, 0.0, temp_limit
        }),
        newDimension(temp_inlet_c, &LINEAR_INTEGRATION, { 
          -temp_limit, 0.0, temp_limit
        })
      ), newDataVec({
         0.0,           temp_limit,   temp_limit*2,
        -temp_limit,    0.0,          temp_limit,
        -temp_limit*2, -temp_limit,   0.0
      }));

      temp_inlet_c->set(0.0);
      temp_manifold_c->set(0.0);
      CHECK(temp_delta->get() == 0.0);
  }
  
  /**
   * Table looks like
   * 
   *    X 1 2
   *      ___
   *  Y 3|1 2|
   *    4|3 4|
   *      ---
   * 
   *    And the coordinates would be x=1.5, y=4.0
   *    This puts you in between 3 and 4, so the result
   *    With linear interpolation is 3.5.
   * 
  */
  TEST_CASE("get() on two dimensional table") {
    double source_x = 1.5;
    double source_y = 4.0;

    v_double* dummy_anchors_x = new v_double();
    dummy_anchors_x->push_back(1);
    dummy_anchors_x->push_back(2);
    v_double* dummy_anchors_y = new v_double();
    dummy_anchors_y->push_back(3);
    dummy_anchors_y->push_back(4);

    v_dimension *dimensions = new std::vector<Dimension*>();
    Variable* variable_x = new Variable(source_x);
    Variable* variable_y = new Variable(source_y);
    dimensions->push_back(new Dimension(variable_x, &LINEAR_INTEGRATION, dummy_anchors_x));
    dimensions->push_back(new Dimension(variable_y, &LINEAR_INTEGRATION, dummy_anchors_y));

    v_double* table_data = new v_double();
    table_data->push_back(1); // 0,0
    table_data->push_back(2); // 1,0
    table_data->push_back(3); // 0,1
    table_data->push_back(4); // 1,1

    std::string table_name = "test_table_2d";
    Table* table = new Table(&table_name, dimensions, table_data);
    double result = table->get();
    CHECK(result == 3.5);
  }

  TEST_CASE("get() on 32x32 two dimensional table") {
    int size = 32;

    v_double* dummy_anchors_x = new v_double();
    for (int x = 0; x < size; x ++) {
      dummy_anchors_x->push_back(x);
    }
    
    v_double* dummy_anchors_y = new v_double();
    for (int y = 0; y < size; y ++) {
      dummy_anchors_y->push_back(y);
    }

    v_dimension *dimensions = new std::vector<Dimension*>();
    TestVariable* variable_x = new TestVariable();
    TestVariable* variable_y = new TestVariable();
    dimensions->push_back(new Dimension(variable_x, &LINEAR_INTEGRATION, dummy_anchors_x));
    dimensions->push_back(new Dimension(variable_y, &LINEAR_INTEGRATION, dummy_anchors_y));

    v_double* table_data = new v_double();
    for (int i = 0; i < size * size; i ++) {
      table_data->push_back(i);
    }

    std::string table_name = "test_table_2d";
    Table* table = new Table(&table_name, dimensions, table_data);
    for (int x = 0; x < size * 16; x ++) { 
      for (int y = 0; y < size; y ++) {
        double x_partial =(double) x / 16.0;
        int x_min = std::floor(x_partial);
        double x_remainder = x_partial - x_min;
        int x_max = std::ceil(x_partial);
        x_max = std::min(x_max, size - 1);
        x_min += (y * size);
        x_max += (y * size);
        variable_x->set(x_partial);
        variable_y->set((double) y);
        double result = table->get();
        double expected = x_min + ((x_max - x_min) * x_remainder);
        CHECK(result == expected);
      }
    }
  }


  TEST_CASE("get() on feedback table") {
    double expected = 1.0;
    double initial_correction = 1.0;

    Variable* dummy_source = new Variable(expected);
    v_double* dummy_anchors = new v_double();
    dummy_anchors->push_back(1);

    v_double* table_data = new v_double();
    table_data->push_back(expected);

    v_double* table_feedback_data = new v_double();
    table_feedback_data->push_back(initial_correction);

    std::string table_name = "test_feedback_table_1d";

    Dimension* dimension_x = new Dimension(dummy_source, &LINEAR_INTEGRATION, dummy_anchors);
    v_dimension *dimensions = new std::vector<Dimension*>();
    dimensions->push_back(dimension_x);

    Variable* real = new Variable(1.0);
    Variable* current = new Variable(2.0);

    FeedbackTable* table = new FeedbackTable(&table_name, dimensions, table_data, real, current, table_feedback_data, 5);

    CHECK(table->get() == expected*initial_correction);
    CHECK(table->get() == expected*initial_correction);
    CHECK(table->get() == expected*initial_correction);
    CHECK(table->get() == expected*initial_correction);

    CHECK(table->get() == expected*table->getCorrectionTable()->getData(0));
  }
}


int main(int argc, char **argv)
{
  doctest::Context context;

  // BEGIN:: PLATFORMIO REQUIRED OPTIONS
  context.setOption("success", true);     // Report successful tests
  context.setOption("no-exitcode", true); // Do not return non-zero code on failed test case
  // END:: PLATFORMIO REQUIRED OPTIONS

  // YOUR CUSTOM DOCTEST OPTIONS

  context.applyCommandLine(argc, argv);
  return context.run();
}