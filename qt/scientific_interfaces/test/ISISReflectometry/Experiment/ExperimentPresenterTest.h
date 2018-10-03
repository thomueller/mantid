#ifndef MANTID_CUSTOMINTERFACES_EXPERIMENTPRESENTERTEST_H_
#define MANTID_CUSTOMINTERFACES_EXPERIMENTPRESENTERTEST_H_

#include "../../../ISISReflectometry/GUI/Experiment/ExperimentPresenter.h"
#include "MockExperimentView.h"

#include <cxxtest/TestSuite.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace MantidQt::CustomInterfaces;
using testing::Mock;
using testing::NiceMock;
using testing::Return;
using testing::_;

class ExperimentPresenterTest : public CxxTest::TestSuite {
  using OptionsRow = std::array<std::string, 8>;
  using OptionsTable = std::vector<OptionsRow>;

public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static ExperimentPresenterTest *createSuite() {
    return new ExperimentPresenterTest();
  }
  static void destroySuite(ExperimentPresenterTest *suite) { delete suite; }

  ExperimentPresenterTest() : m_view() {}

  void testAllWidgetsAreEnabledWhenReductionPaused() {
    auto presenter = makePresenter();

    EXPECT_CALL(m_view, enableAll()).Times(1);
    presenter.onReductionPaused();

    verifyAndClear();
  }

  void testAllWidgetsAreDisabledWhenReductionResumed() {
    auto presenter = makePresenter();

    EXPECT_CALL(m_view, disableAll()).Times(1);
    presenter.onReductionResumed();

    verifyAndClear();
  }

  void testModelUpdatedWhenAnalysisModeChanged() {
    auto presenter = makePresenter();

    expectViewReturnsSumInLambdaDefaults();
    expectViewReturnsDefaultPolarizationCorrectionType();
    EXPECT_CALL(m_view, getAnalysisMode())
        .WillOnce(Return(std::string("MultiDetectorAnalysis")));
    presenter.notifySettingsChanged();

    TS_ASSERT_EQUALS(presenter.experiment().analysisMode(),
                     AnalysisMode::MultiDetector);
    verifyAndClear();
  }

  void testModelUpdatedWhenSummationTypeChanged() {
    auto presenter = makePresenter();

    expectViewReturnsDefaultAnalysisMode();
    expectViewReturnsSumInQDefaults();
    expectViewReturnsDefaultPolarizationCorrectionType();
    presenter.notifySummationTypeChanged();

    TS_ASSERT_EQUALS(presenter.experiment().summationType(),
                     SummationType::SumInQ);
    verifyAndClear();
  }

  void testReductionTypeDisabledWhenChangeToSumInLambda() {
    auto presenter = makePresenter();

    expectViewReturnsDefaultAnalysisMode();
    expectViewReturnsSumInLambdaDefaults();
    expectViewReturnsDefaultPolarizationCorrectionType();
    EXPECT_CALL(m_view, disableReductionType()).Times(1);
    presenter.notifySummationTypeChanged();

    verifyAndClear();
  }

  void testReductionTypeEnbledWhenChangeToSumInQ() {
    auto presenter = makePresenter();

    expectViewReturnsDefaultAnalysisMode();
    expectViewReturnsSumInQDefaults();
    expectViewReturnsDefaultPolarizationCorrectionType();
    EXPECT_CALL(m_view, enableReductionType()).Times(1);
    presenter.notifySummationTypeChanged();

    verifyAndClear();
  }

  void testSetPolarizationCorrectionsUpdatesModel() {
    auto presenter = makePresenter();
    PolarizationCorrections polCorr(PolarizationCorrectionType::PA, 1.2, 1.3,
                                    2.4, 2.5);

    expectViewReturnsDefaultAnalysisMode();
    expectViewReturnsSumInLambdaDefaults();
    EXPECT_CALL(m_view, getPolarizationCorrectionType()).WillOnce(Return("PA"));
    EXPECT_CALL(m_view, getCRho()).WillOnce(Return(polCorr.cRho().get()));
    EXPECT_CALL(m_view, getCAlpha()).WillOnce(Return(polCorr.cAlpha().get()));
    EXPECT_CALL(m_view, getCAp()).WillOnce(Return(polCorr.cAp().get()));
    EXPECT_CALL(m_view, getCPp()).WillOnce(Return(polCorr.cPp().get()));
    presenter.notifySettingsChanged();

    TS_ASSERT_EQUALS(presenter.experiment().polarizationCorrections(), polCorr);
    verifyAndClear();
  }

  void testSettingPolarizationCorrectionsToNoneDisablesInputs() {
    runWithPolarizationCorrectionInputsDisabled("None");
  }

  void testSetPolarizationCorrectionsToParameterFileDisablesInputs() {
    runWithPolarizationCorrectionInputsDisabled("ParameterFile");
  }

  void testSettingPolarizationCorrectionsToPAEnablesInputs() {
    runWithPolarizationCorrectionInputsEnabled("PA");
  }

  void testSettingPolarizationCorrectionsToPNREnablesInputs() {
    runWithPolarizationCorrectionInputsEnabled("PNR");
  }

  void testSetValidTransmissionRunRange() {
    RangeInLambda range(7.2, 10);
    runTestForValidTransmissionRunRange(range, range);
  }

  void testTransmissionRunRangeIsInvalidIfStartGreaterThanEnd() {
    RangeInLambda range(10.2, 7.1);
    runTestForInvalidTransmissionRunRange(range);
  }

  void testTransmissionRunRangeIsInvalidIfZeroLength() {
    RangeInLambda range(7.1, 7.1);
    runTestForInvalidTransmissionRunRange(range);
  }

  void testTransmissionRunRangeIsValidIfStartUnset() {
    RangeInLambda range(0.0, 7.1);
    runTestForValidTransmissionRunRange(range, range);
  }

  void testTransmissionRunRangeIsValidIfEndUnset() {
    RangeInLambda range(5, 0.0);
    runTestForValidTransmissionRunRange(range, range);
  }

  void testTransmissionRunRangeIsValidButNotUpdatedIfUnset() {
    RangeInLambda range(0.0, 0.0);
    runTestForValidTransmissionRunRange(range, boost::none);
  }

  void testSetStitchOptions() {
    auto presenter = makePresenter();
    auto const optionsString = "Params=0.02";
    std::map<std::string, std::string> optionsMap = {{"Params", "0.02"}};

    expectViewReturnsDefaultValues();
    EXPECT_CALL(m_view, getStitchOptions()).WillOnce(Return(optionsString));
    EXPECT_CALL(m_view, showStitchParametersValid());
    presenter.notifySettingsChanged();

    TS_ASSERT_EQUALS(presenter.experiment().stitchParameters(), optionsMap);
    verifyAndClear();
  }

  void testSetStitchOptionsInvalid() {
    auto presenter = makePresenter();
    auto const optionsString = "0.02";
    std::map<std::string, std::string> emptyOptionsMap;

    expectViewReturnsDefaultValues();
    EXPECT_CALL(m_view, getStitchOptions()).WillOnce(Return(optionsString));
    EXPECT_CALL(m_view, showStitchParametersInvalid());
    presenter.notifySettingsChanged();

    TS_ASSERT_EQUALS(presenter.experiment().stitchParameters(),
                     emptyOptionsMap);
    verifyAndClear();
  }

  void testNewPerAngleDefaultsRequested() {
    auto presenter = makePresenter();

    // row should be added to view
    EXPECT_CALL(m_view, addPerThetaDefaultsRow());
    // new value should be requested from view to update model
    expectViewReturnsDefaultValues();
    EXPECT_CALL(m_view, getPerAngleOptions()).Times(1);
    presenter.notifyNewPerAngleDefaultsRequested();

    verifyAndClear();
  }

  void testRemovePerAngleDefaultsRequested() {
    auto presenter = makePresenter();

    int const indexToRemove = 0;
    // row should be removed from view
    EXPECT_CALL(m_view, removePerThetaDefaultsRow(indexToRemove)).Times(1);
    // new value should be requested from view to update model
    expectViewReturnsDefaultValues();
    EXPECT_CALL(m_view, getPerAngleOptions()).Times(1);
    presenter.notifyRemovePerAngleDefaultsRequested(indexToRemove);

    verifyAndClear();
  }

  void testChangingPerAngleDefaultsUpdatesModel() {
    auto presenter = makePresenter();

    auto const row = 1;
    auto const column = 2;
    OptionsTable const optionsTable = {optionsRowWithFirstAngle(),
                                       optionsRowWithSecondAngle()};
    expectViewReturnsDefaultValues();
    EXPECT_CALL(m_view, getPerAngleOptions()).WillOnce(Return(optionsTable));
    presenter.notifyPerAngleDefaultsChanged(row, column);

    // Check the model contains the per-theta defaults returned by the view
    auto const perThetaDefaults = presenter.experiment().perThetaDefaults();
    TS_ASSERT_EQUALS(perThetaDefaults.size(), 2);
    TS_ASSERT_EQUALS(perThetaDefaults[0], defaultsWithFirstAngle());
    TS_ASSERT_EQUALS(perThetaDefaults[1], defaultsWithSecondAngle());
    verifyAndClear();
  }

  // TODO
  void testMultipleUniqueAnglesAreValid() {
    OptionsTable const optionsTable = {optionsRowWithFirstAngle(),
                                       optionsRowWithSecondAngle()};
    runTestForValidPerAngleOptions(optionsTable);
  }

  void testMultipleNonUniqueAnglesAreInvalid() {
    OptionsTable const optionsTable = {optionsRowWithFirstAngle(),
                                       optionsRowWithFirstAngle()};
    runTestForNonUniqueAngles(optionsTable);
  }

  void testSingleWildcardRowIsValid() {
    OptionsTable const optionsTable = {optionsRowWithWildcard()};
    runTestForValidPerAngleOptions(optionsTable);
  }

  void testAngleAndWildcardRowAreValid() {
    OptionsTable const optionsTable = {optionsRowWithFirstAngle(),
                                       optionsRowWithWildcard()};
    runTestForValidPerAngleOptions(optionsTable);
  }

  void testMultipleWildcardRowsAreInvalid() {
    OptionsTable const optionsTable = {optionsRowWithWildcard(),
                                       optionsRowWithWildcard()};
    runTestForNonUniqueAngles(optionsTable);
  }

  void testSetFirstTransmissionRun() {}

  void testSetSecondTransmissionRun() {}

  void testSetBothTransmissionRuns() {}

  void testSetQMin() {}

  void testSetQMinInvalid() {}

  void testSetQMax() {}

  void testSetQMaxInvalid() {}

  void testSetQStep() {}

  void testSetQStepInvalid() {}

  void testSetScale() {}

  void testSetScaleInvalid() {}

  void testSetProcessingInstructions() {}

  void testSetProcessingInstructionsInvalid() {}

private:
  NiceMock<MockExperimentView> m_view;
  double m_thetaTolerance{0.01};

  Experiment makeModel() {
    auto polarizationCorrections =
        PolarizationCorrections(PolarizationCorrectionType::None, boost::none,
                                boost::none, boost::none, boost::none);
    auto transmissionRunRange = boost::none;
    auto stitchParameters = std::map<std::string, std::string>();
    auto perThetaDefaults = std::vector<PerThetaDefaults>();
    return Experiment(AnalysisMode::PointDetector, ReductionType::Normal,
                      SummationType::SumInLambda,
                      std::move(polarizationCorrections),
                      std::move(transmissionRunRange),
                      std::move(stitchParameters), std::move(perThetaDefaults));
  }

  ExperimentPresenter makePresenter() {
    // The presenter gets values from the view on construction so the view must
    // return something sensible
    expectViewReturnsDefaultValues();
    auto presenter =
        ExperimentPresenter(&m_view, makeModel(), m_thetaTolerance);
    verifyAndClear();
    return presenter;
  }

  void verifyAndClear() {
    TS_ASSERT(Mock::VerifyAndClearExpectations(&m_view));
  }

  void expectViewReturnsDefaultAnalysisMode() {
    EXPECT_CALL(m_view, getAnalysisMode())
        .WillOnce(Return(std::string("PointDetectorAnalysis")));
  }

  void expectViewReturnsSumInLambdaDefaults() {
    EXPECT_CALL(m_view, getSummationType())
        .WillOnce(Return(std::string("SumInLambda")));
    EXPECT_CALL(m_view, getReductionType())
        .WillOnce(Return(std::string("Normal")));
  }

  void expectViewReturnsSumInQDefaults() {
    EXPECT_CALL(m_view, getSummationType())
        .WillOnce(Return(std::string("SumInQ")));
    EXPECT_CALL(m_view, getReductionType())
        .WillOnce(Return(std::string("DivergentBeam")));
  }

  void expectViewReturnsDefaultPolarizationCorrectionType() {
    EXPECT_CALL(m_view, getPolarizationCorrectionType())
        .WillOnce(Return(std::string("None")));
  }

  void expectViewReturnsDefaultValues() {
    expectViewReturnsDefaultAnalysisMode();
    expectViewReturnsSumInLambdaDefaults();
    expectViewReturnsDefaultPolarizationCorrectionType();
  }

  void runWithPolarizationCorrectionInputsDisabled(std::string const &type) {
    auto presenter = makePresenter();

    expectViewReturnsDefaultAnalysisMode();
    expectViewReturnsSumInLambdaDefaults();
    EXPECT_CALL(m_view, getPolarizationCorrectionType()).WillOnce(Return(type));
    EXPECT_CALL(m_view, disablePolarizationCorrectionInputs()).Times(1);
    EXPECT_CALL(m_view, getCRho()).Times(0);
    EXPECT_CALL(m_view, getCAlpha()).Times(0);
    EXPECT_CALL(m_view, getCAp()).Times(0);
    EXPECT_CALL(m_view, getCPp()).Times(0);
    presenter.notifySettingsChanged();

    verifyAndClear();
  }

  void runWithPolarizationCorrectionInputsEnabled(std::string const &type) {
    auto presenter = makePresenter();

    expectViewReturnsDefaultAnalysisMode();
    expectViewReturnsSumInLambdaDefaults();
    EXPECT_CALL(m_view, getPolarizationCorrectionType()).WillOnce(Return(type));
    EXPECT_CALL(m_view, enablePolarizationCorrectionInputs()).Times(1);
    EXPECT_CALL(m_view, getCRho()).Times(1);
    EXPECT_CALL(m_view, getCAlpha()).Times(1);
    EXPECT_CALL(m_view, getCAp()).Times(1);
    EXPECT_CALL(m_view, getCPp()).Times(1);
    presenter.notifySettingsChanged();

    verifyAndClear();
  }

  void runTestForValidTransmissionRunRange(
      RangeInLambda const &range,
      boost::optional<RangeInLambda> const &result) {
    auto presenter = makePresenter();
    expectViewReturnsDefaultValues();
    EXPECT_CALL(m_view, getTransmissionStartOverlap())
        .WillOnce(Return(range.min()));
    EXPECT_CALL(m_view, getTransmissionEndOverlap())
        .WillOnce(Return(range.max()));
    EXPECT_CALL(m_view, showTransmissionRangeValid()).Times(1);
    presenter.notifySettingsChanged();
    TS_ASSERT_EQUALS(presenter.experiment().transmissionRunRange(), result);
    verifyAndClear();
  }

  void runTestForInvalidTransmissionRunRange(RangeInLambda const &range) {
    auto presenter = makePresenter();
    expectViewReturnsDefaultValues();
    EXPECT_CALL(m_view, getTransmissionStartOverlap())
        .WillOnce(Return(range.min()));
    EXPECT_CALL(m_view, getTransmissionEndOverlap())
        .WillOnce(Return(range.max()));
    EXPECT_CALL(m_view, showTransmissionRangeInvalid()).Times(1);
    presenter.notifySettingsChanged();
    TS_ASSERT_EQUALS(presenter.experiment().transmissionRunRange(),
                     boost::none);
    verifyAndClear();
  }

  // These functions create various rows in the per-theta defaults tables,
  // either as an input array of strings or an output model
  OptionsRow optionsRowWithFirstAngle() { return {"0.5", "13463"}; }
  PerThetaDefaults defaultsWithFirstAngle() {
    return PerThetaDefaults(0.5, std::make_pair("13463", ""), boost::none,
                            boost::none, boost::none);
  }

  OptionsRow optionsRowWithSecondAngle() {
    return {"2.3", "13463", "13464"};
  }
  PerThetaDefaults defaultsWithSecondAngle() {
    return PerThetaDefaults(2.3, std::make_pair("13463", "13464"), boost::none,
                            boost::none, boost::none);
  }
  OptionsRow optionsRowWithWildcard() {
    return {"", "13463", "13464"};
  }

  // TODO
  void runTestForValidPerAngleOptions(OptionsTable const &optionsTable) {
    auto presenter = makePresenter();
    expectViewReturnsDefaultValues();
    EXPECT_CALL(m_view, getPerAngleOptions()).WillOnce(Return(optionsTable));
    EXPECT_CALL(m_view, showAllPerAngleOptionsAsValid()).Times(1);
    presenter.notifyPerAngleDefaultsChanged(1, 1);
    verifyAndClear();
  }

  void runTestForNonUniqueAngles(OptionsTable const &optionsTable) {
    auto presenter = makePresenter();
    EXPECT_CALL(m_view, getPerAngleOptions()).WillOnce(Return(optionsTable));
    EXPECT_CALL(m_view, showPerAngleThetasNonUnique(m_thetaTolerance)).Times(1);
    presenter.notifyPerAngleDefaultsChanged(0, 0);
    verifyAndClear();
  }
};

#endif // MANTID_CUSTOMINTERFACES_EXPERIMENTPRESENTERTEST_H_
