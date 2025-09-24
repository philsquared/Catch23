//
// Created by Phil Nash on 23/07/2025.
//

#ifndef CATCHKIT_REPORT_ON_H
#define CATCHKIT_REPORT_ON_H

namespace CatchKit {

    enum class ReportOn {
        Nothing = 0,
        PassingTests = 1,
        FailingTests = 2,
        AllResults = PassingTests | FailingTests,
    };

    constexpr ReportOn operator&(ReportOn lhs, ReportOn rhs) {
        return static_cast<ReportOn>(
            static_cast<std::underlying_type_t<ReportOn>>(lhs) &
            static_cast<std::underlying_type_t<ReportOn>>(rhs)
        );
    }
    inline bool report_on_passing(ReportOn report_on) {
        return (report_on & ReportOn::PassingTests) == ReportOn::PassingTests;
    }
    inline bool report_on_failing(ReportOn report_on) {
        return (report_on & ReportOn::FailingTests) == ReportOn::FailingTests;
    }

} // namespace CatchKit

#endif // CATCHKIT_REPORT_ON_H
