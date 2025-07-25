//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "../../unit-tests/SuppressionMacros.hpp"
#include "mimic++/Mock.hpp"

#define BOOST_TEST_MODULE BoostAdapterTest
#include "mimic++_ext/adapters/BoostTest.hpp"

BOOST_AUTO_TEST_SUITE(SuccessSuite)

BOOST_AUTO_TEST_CASE(ReportSuccess)
{
    mimicpp::reporting::detail::boost_test::send_success("Report Success");
}

BOOST_AUTO_TEST_CASE(ReportWarning)
{
    mimicpp::reporting::detail::boost_test::send_warning("Report Warning");
}

BOOST_AUTO_TEST_SUITE_END()

// This tests will fail. ctest has appropriate properties set, thus should be reported as success
BOOST_AUTO_TEST_SUITE(FailureSuite)

START_WARNING_SUPPRESSION
SUPPRESS_UNREACHABLE_CODE

BOOST_AUTO_TEST_CASE(ReportFail)
{
    mimicpp::reporting::detail::boost_test::send_fail("Report fail");
}

STOP_WARNING_SUPPRESSION

BOOST_AUTO_TEST_SUITE_END()
