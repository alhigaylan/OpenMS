// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2023.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Jihyung Kim$
// $Authors: Jihyung Kim$
// --------------------------------------------------------------------------

#include <OpenMS/CONCEPT/ClassTest.h>
#include <OpenMS/test_config.h>

///////////////////////////
#include <OpenMS/ANALYSIS/TOPDOWN/FLASHDeconvAlgorithm.h>
#include <OpenMS/FORMAT/MzMLFile.h>
///////////////////////////

using namespace OpenMS;
using namespace std;

START_TEST(FLASHDeconvAlgorithm, "$Id$")

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

FLASHDeconvAlgorithm* ptr = 0;
FLASHDeconvAlgorithm* null_ptr = 0;
START_SECTION(FLASHDeconvAlgorithm())
{
  ptr = new FLASHDeconvAlgorithm();
  TEST_NOT_EQUAL(ptr, null_ptr)
}
END_SECTION

START_SECTION(~FLASHDeconvAlgorithm())
{
  delete ptr;
}
END_SECTION


/// < public methods without tests >
/// - default constructors and operators are not used (copy, move, assignment)
/// - setTargetMasses : only private member (which can not be accessed) is affected
/// - getDecoyDeconvolvedSpectrum, isDecoy, addPreviouslyDeconvolvedMonoMass, clearPreviouslyDeconvolvedMonoMasses: under development
/// - getAvgPPMError

FLASHDeconvAlgorithm fd_algo = FLASHDeconvAlgorithm();
Param fd_param;
fd_param.setValue("min_charge", 5);
fd_param.setValue("max_charge", 20);

START_SECTION((static int getNominalMass(const double mass)))
{
  double tmp_mass1 = 10000;
  double tmp_mass2 = 25000;

  TEST_EQUAL(fd_algo.getNominalMass(tmp_mass1), 9995);
  TEST_EQUAL(fd_algo.getNominalMass(tmp_mass2), 24987);
}
END_SECTION

START_SECTION((void calculateAveragine(const bool use_RNA_averagine)))
{
  fd_param.setValue("max_mass", 2000.);
  fd_algo.setParameters(fd_param);

  FLASHDeconvAlgorithm tmp_algo = FLASHDeconvAlgorithm();
  fd_param.setValue("max_mass", 100.);
  tmp_algo.setParameters(fd_param);

  fd_algo.calculateAveragine(false);
  tmp_algo.calculateAveragine(true);
  const auto &precalculated_avg = fd_algo.getAveragine();
  const auto &precalculated_avg_tmp = tmp_algo.getAveragine();

  TEST_EQUAL(precalculated_avg.getMaxIsotopeIndex(), 8);
  TEST_EQUAL(precalculated_avg.getApexIndex(50), 0);
  TOLERANCE_ABSOLUTE(0.1)
  TEST_REAL_SIMILAR(precalculated_avg.getAverageMassDelta(50), 0.0296591659229435);

  TEST_EQUAL(precalculated_avg_tmp.getMaxIsotopeIndex(), 3);
  TEST_EQUAL(precalculated_avg_tmp.getApexIndex(50), 0);
  TEST_REAL_SIMILAR(precalculated_avg_tmp.getAverageMassDelta(50), 0.025145817950033234);
}
END_SECTION

START_SECTION((PrecalculatedAveragine& getAveragine()))
{
  const auto &precalculated_avg = fd_algo.getAveragine();

  TEST_EQUAL(precalculated_avg.getMaxIsotopeIndex(), 8);
  TEST_EQUAL(precalculated_avg.getApexIndex(50), 0);
  TEST_REAL_SIMILAR(precalculated_avg.getAverageMassDelta(50), 0.0296591659229435);
}
END_SECTION

// load test data
PeakMap input;
MzMLFile().load(OPENMS_GET_TEST_DATA_PATH("FLASHDeconv_sample_input1.mzML"), input);

// resetting fd_algo based on the test data
fd_param.setValue("max_mass", 50000.);
fd_algo.setParameters(fd_param);
fd_algo.calculateAveragine(false);

START_SECTION(DeconvolvedSpectrum& getDeconvolvedSpectrum())
{
  std::vector<DeconvolvedSpectrum> survey_specs;
  const std::map<int, std::vector<std::vector<float>>> null_map;

  fd_algo.performSpectrumDeconvolution(input[3], survey_specs, 4, null_map);

  DeconvolvedSpectrum d_ms1_spec = fd_algo.getDeconvolvedSpectrum();
  TEST_EQUAL(d_ms1_spec.size(), 1);
}
END_SECTION

START_SECTION((DeconvolvedSpectrum& performSpectrumDeconvolution(const MSSpectrum &spec, const std::vector< DeconvolvedSpectrum > &survey_scans, const int scan_number, const bool write_detail, const std::map< int, std::vector< std::vector< double >>> &precursor_map_for_FLASHIda)))
{
  std::vector<DeconvolvedSpectrum> survey_specs;
  const std::map<int, std::vector<std::vector<float>>> null_map;

  fd_algo.performSpectrumDeconvolution(input[3], survey_specs, 4, null_map);
  DeconvolvedSpectrum d_ms1_spec = fd_algo.getDeconvolvedSpectrum();
  survey_specs.push_back(d_ms1_spec);
  fd_algo.performSpectrumDeconvolution(input[5], survey_specs, 6, null_map);
  DeconvolvedSpectrum d_ms2_spec = fd_algo.getDeconvolvedSpectrum();
  TEST_EQUAL(d_ms1_spec.getScanNumber(), 4);
  TEST_EQUAL(d_ms1_spec.size(), 1);
  Precursor precursor = d_ms2_spec.getPrecursor();
  TOLERANCE_ABSOLUTE(1);
  TEST_EQUAL(d_ms1_spec.getPrecursorPeakGroup().size(), 0);
  TEST_EQUAL(d_ms2_spec.getPrecursorPeakGroup().size(), 0);
  TEST_EQUAL(precursor.getCharge(), 9);
  TOLERANCE_ABSOLUTE(100);
  TEST_REAL_SIMILAR(precursor.getIntensity(), 12293.4);
}
END_SECTION



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
END_TEST
