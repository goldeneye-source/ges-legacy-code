#ifndef SDK_LISTENER_H
#define SDK_LISTENER_H

using namespace testing;

// This class implements the TestEventListener interface.
//
// Class SDKUnitTestResultPrinter is copyable.
class SDKUnitTestListener : public EmptyTestEventListener {
 public:
  SDKUnitTestListener() {}
  static void PrintTestName(const char * test_case, const char * test) {
    Msg("%s.%s", test_case, test);
  }

  // The following methods override what's in the TestEventListener class.
  virtual void OnTestIterationStart(const UnitTest& unit_test, int iteration);
  virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test);
  virtual void OnTestCaseStart(const TestCase& test_case);
  virtual void OnTestStart(const TestInfo& test_info);
  virtual void OnTestPartResult(const TestPartResult& result);
  virtual void OnTestEnd(const TestInfo& test_info);
  virtual void OnTestCaseEnd(const TestCase& test_case);
  virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test);
  virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration);

 private:
  static void PrintFailedTests(const UnitTest& unit_test);
};

#endif
