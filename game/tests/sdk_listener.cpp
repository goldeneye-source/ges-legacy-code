#include "cbase.h"
#include "gtest/gtest.h"

#include "sdk_listener.h"

using namespace testing;

static const char kTypeParamLabel[] = "TypeParam";
static const char kValueParamLabel[] = "GetParam()";
static const char kUniversalFilter[] = "*";

// Formats a countable noun.  Depending on its quantity, either the
// singular form or the plural form is used. e.g.
static std::string FormatCountableNoun(int count, const char * singular_form, const char * plural_form) {
	return (Message() 
		<< count << " " << (count == 1 ? singular_form : plural_form) ).GetString();
}

// Formats the count of tests.
static std::string FormatTestCount(int test_count) {
	return FormatCountableNoun(test_count, "test", "tests");
}

// Formats the count of test cases.
static std::string FormatTestCaseCount(int test_case_count) {
	return FormatCountableNoun(test_case_count, "test case", "test cases");
}

// Converts a TestPartResult::Type enum to human-friendly string
// representation.  Both kNonFatalFailure and kFatalFailure are translated
// to "Failure", as the user usually doesn't care about the difference
// between the two when viewing the test result.
static const char * TestPartResultTypeToString(TestPartResult::Type type) {
	switch (type) {
	case TestPartResult::kSuccess:
		return "Success";

	case TestPartResult::kNonFatalFailure:
	case TestPartResult::kFatalFailure:
#ifdef _MSC_VER
		return "error: ";
#else
		return "Failure\n";
#endif
	default:
		return "Unknown result type";
	}
}

// Prints a TestPartResult to an std::string.
static std::string PrintTestPartResultToString(const TestPartResult& test_part_result) {
	return (Message()
		<< internal::FormatFileLocation(test_part_result.file_name(),
		test_part_result.line_number())
		<< " " << TestPartResultTypeToString(test_part_result.type())
		<< test_part_result.message()).GetString();
}

// Fired before each iteration of tests starts.
void SDKUnitTestListener::OnTestIterationStart(const UnitTest& unit_test, int iteration) {
	if (GTEST_FLAG(repeat) != 1)
		Msg("\nRepeating all tests (iteration %d) . . .\n\n", iteration + 1);

	const char* const filter = GTEST_FLAG(filter).c_str();

	// Prints the filter if it's not *.  This reminds the user that some
	// tests may be skipped.
	if (!internal::String::CStringEquals(filter, kUniversalFilter)) {
		Msg( "Note: %s filter = %s\n", GTEST_NAME_, filter );
	}

	if (GTEST_FLAG(shuffle)) {
		Msg( "Note: Randomizing tests' orders with a seed of %d .\n",
			unit_test.random_seed());
	}

	Msg( "[==========] ");
	Msg("Running %s from %s.\n",
		FormatTestCount(unit_test.test_to_run_count()).c_str(),
		FormatTestCaseCount(unit_test.test_case_to_run_count()).c_str());
}

void SDKUnitTestListener::OnEnvironmentsSetUpStart(const UnitTest& /*unit_test*/) {
		Msg( "[----------] ");
		Msg("Global test environment set-up.\n");
}

void SDKUnitTestListener::OnTestCaseStart(const TestCase& test_case) {
	const std::string counts =
		FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
	Msg( "[----------] ");
	Msg("%s from %s", counts.c_str(), test_case.name());
	if (test_case.type_param() == NULL) {
		Msg("\n");
	} else {
		Msg(", where %s = %s\n", kTypeParamLabel, test_case.type_param());
	}
}

void SDKUnitTestListener::OnTestStart(const TestInfo& test_info) {
	Msg( "[ RUN      ] ");
	PrintTestName(test_info.test_case_name(), test_info.name());
	Msg("\n");
}

// Called after an assertion failure.
void SDKUnitTestListener::OnTestPartResult(const TestPartResult& result) {
	// If the test part succeeded, we don't need to do anything.
	if (result.type() == TestPartResult::kSuccess)
		return;

	// Print failure message from the assertion (e.g. expected this and got that).
	std::string res = PrintTestPartResultToString(result);
	Msg( "%s\n", res.c_str() );
}

void SDKUnitTestListener::OnTestEnd(const TestInfo& test_info) {
	if (test_info.result()->Passed()) {
		Msg( "[       OK ] ");
	} else {
		Msg( "[  FAILED  ] ");
	}

	PrintTestName(test_info.test_case_name(), test_info.name());

	if (GTEST_FLAG(print_time)) {
		Msg(" (%s ms)\n", internal::StreamableToString(
			test_info.result()->elapsed_time()).c_str());
	} else {
		Msg("\n");
	}
}

void SDKUnitTestListener::OnTestCaseEnd(const TestCase& test_case) {
	if (!GTEST_FLAG(print_time)) return;

	const std::string counts = FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
	Msg( "[----------] ");
	Msg("%s from %s (%s ms total)\n\n",
		counts.c_str(), test_case.name(),
		internal::StreamableToString(test_case.elapsed_time()).c_str());
}

void SDKUnitTestListener::OnEnvironmentsTearDownStart(
	const UnitTest& /*unit_test*/) {
		Msg( "[----------] ");
		Msg("Global test environment tear-down\n");
}

// Internal helper for printing the list of failed tests.
void SDKUnitTestListener::PrintFailedTests(const UnitTest& unit_test) {
	const int failed_test_count = unit_test.failed_test_count();
	if (failed_test_count == 0) {
		return;
	}

	for (int i = 0; i < unit_test.total_test_case_count(); ++i) {
		const TestCase& test_case = *unit_test.GetTestCase(i);
		if (!test_case.should_run() || (test_case.failed_test_count() == 0)) {
			continue;
		}
		for (int j = 0; j < test_case.total_test_count(); ++j) {
			const TestInfo& test_info = *test_case.GetTestInfo(j);
			if (!test_info.should_run() || test_info.result()->Passed()) {
				continue;
			}
			Msg( "[  FAILED  ] ");
			Msg("%s.%s", test_case.name(), test_info.name());
			Msg("\n");
		}
	}
}

void SDKUnitTestListener::OnTestIterationEnd(const UnitTest& unit_test, int /*iteration*/) {
	Msg( "[==========] ");
	Msg("%s from %s ran.",
		FormatTestCount(unit_test.test_to_run_count()).c_str(),
		FormatTestCaseCount(unit_test.test_case_to_run_count()).c_str());
	if (GTEST_FLAG(print_time)) {
		Msg(" (%s ms total)",
			internal::StreamableToString(unit_test.elapsed_time()).c_str());
	}
	Msg("\n");
	Msg( "[  PASSED  ] ");
	Msg("%s.\n", FormatTestCount(unit_test.successful_test_count()).c_str());

	int num_failures = unit_test.failed_test_count();
	if (!unit_test.Passed()) {
		const int failed_test_count = unit_test.failed_test_count();
		Msg( "[  FAILED  ] ");
		Msg("%s, listed below:\n", FormatTestCount(failed_test_count).c_str());
		PrintFailedTests(unit_test);
		Msg("\n%2d FAILED %s\n", num_failures, num_failures == 1 ? "TEST" : "TESTS");
	}

	int num_disabled = unit_test.disabled_test_count();
	if (num_disabled && !GTEST_FLAG(also_run_disabled_tests)) {
		if (!num_failures) {
			Msg("\n");  // Add a spacer if no FAILURE banner is displayed.
		}
		Msg( "  YOU HAVE %d DISABLED %s\n\n",
			num_disabled,
			num_disabled == 1 ? "TEST" : "TESTS");
	}
}
