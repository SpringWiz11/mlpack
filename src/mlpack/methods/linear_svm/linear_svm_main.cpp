/**
 * @file linear_svm_main.cpp
 * @author Yashwant Singh Parihar
 *
 * Main executable for linear svm.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/prereqs.hpp>
#include <mlpack/core/util/cli.hpp>
#include <mlpack/core.hpp>
#include <mlpack/core/util/mlpack_main.hpp>

#include "linear_svm.hpp"

#include <ensmallen.hpp>

using namespace std;
using namespace mlpack;
using namespace mlpack::svm;
using namespace mlpack::util;

PROGRAM_INFO("Linear SVM is an L2-regularized support vector machine",
    // Short description.
    "An implementation of linear SVM for multiclass classification. "
    "Given labeled data, a model can be trained and saved for "
    "future use; or, a pre-trained model can be used to classify new points.",
    // Long description.
    "An implementation of linear SVMs that uses either L-BFGS or parallel SGD"
    " (stochastic gradient descent) to train the model."
    "\n\n"
    "This program allows loading a linear SVM model (via the " +
    PRINT_PARAM_STRING("input_model") + " parameter) "
    "or training a linear SVM model given training data (specified "
    "with the " + PRINT_PARAM_STRING("training") + " parameter), or both "
    "those things at once.  In addition, this program allows classification on "
    "a test dataset (specified with the " + PRINT_PARAM_STRING("test") + " "
    "parameter) and the classification results may be saved with the " +
    PRINT_PARAM_STRING("predictions") + " output parameter."
    " The trained linear SVM model may be saved using the " +
    PRINT_PARAM_STRING("output_model") + " output parameter."
    "\n\n"
    "The training data, if specified, may have class labels as its last "
    "dimension.  Alternately, the " + PRINT_PARAM_STRING("labels") + " "
    "parameter may be used to specify a separate matrix of labels."
    "\n\n"
    "When a model is being trained, there are many options.  L2 regularization "
    "(to prevent overfitting) can be specified with the " +
    PRINT_PARAM_STRING("lambda") + " option, and the number of classes can be "
    "manually specified with the " + PRINT_PARAM_STRING("number_of_classes") +
    "and if an intercept term is not desired in the model, the " +
    PRINT_PARAM_STRING("no_intercept") + " parameter can be specified."
    "Margin of difference between correct class and other classes can "
    "be specified with the" + PRINT_PARAM_STRING("delta") + "option."
    "The optimizer used to train the model can be specified with the " +
    PRINT_PARAM_STRING("optimizer") + " parameter.  Available options are "
    "'psgd' (parallel stochastic gradient descent) and 'lbfgs' (the L-BFGS"
    " optimizer).  There are also various parameters for the optimizer; the " +
    PRINT_PARAM_STRING("max_iterations") + " parameter specifies the maximum "
    "number of allowed iterations, and the " +
    PRINT_PARAM_STRING("tolerance") + " parameter specifies the tolerance for "
    "convergence.  For the parallel SGD optimizer, the " +
    PRINT_PARAM_STRING("step_size") + " parameter controls the step size taken "
    "at each iteration by the optimizer and the maximum number of epochs "
    "(specified with " + PRINT_PARAM_STRING("epochs") + "). If the "
    "objective function for your data is oscillating between Inf and 0, the "
    "step size is probably too large.  There are more parameters for the "
    "optimizers, but the C++ interface must be used to access these."
    "\n\n"
    "Optionally, the model can be used to predict the labels for another "
    "matrix of data points, if " + PRINT_PARAM_STRING("test") + " is "
    "specified.  The " + PRINT_PARAM_STRING("test") + " parameter can be "
    "specified without the " + PRINT_PARAM_STRING("training") + " parameter, "
    "so long as an existing linear SVM model is given with the " +
    PRINT_PARAM_STRING("input_model") + " parameter.  The output predictions "
    "from the linear SVM model may be saved with the " +
    PRINT_PARAM_STRING("predictions") + " parameter." +
    "\n\n"
    "As an example, to train a LinaerSVM model on the data '" +
    PRINT_DATASET("data") + "' with labels '" + PRINT_DATASET("labels") + "' "
    "with L2 regularization of 0.1, saving the model to '" +
    PRINT_MODEL("lsvm_model") + "', the following command may be used:"
    "\n\n" +
    PRINT_CALL("linear_svm", "training", "data", "labels", "labels",
        "lambda", 0.1, "delta", 1.0, "number_of_classes", 0,
        "output_model", "lsvm_model") +
    "\n\n"
    "Then, to use that model to predict classes for the dataset '" +
    PRINT_DATASET("test") + "', storing the output predictions in '" +
    PRINT_DATASET("predictions") + "', the following command may be used: "
    "\n\n" +
    PRINT_CALL("linear_svm", "input_model", "lsvm_model", "test", "test",
        "predictions", "predictions"),
    SEE_ALSO("@random_forest", "#random_forest"),
    SEE_ALSO("@logistic_regression", "#logistic_regression"),
    SEE_ALSO("LinearSVM on Wikipedia",
        "https://en.wikipedia.org/wiki/Support-vector_machine"),
    SEE_ALSO("mlpack::svm::LinearSVM C++ class documentation",
        "@doxygen/classmlpack_1_1svm_1_1LinearSVM.html"));

// Training parameters.
PARAM_MATRIX_IN("training", "A matrix containing the training set (the matrix "
    "of predictors, X).", "t");
PARAM_UROW_IN("labels", "A matrix containing labels (0 or 1) for the points "
    "in the training set (y).", "l");

// Optimizer parameters.
PARAM_DOUBLE_IN("lambda", "L2-regularization parameter for training.", "L",
    0.0001);
PARAM_DOUBLE_IN("delta", "Margin of difference between correct class and other "
    "classes.", "d", 1.0);
PARAM_INT_IN("number_of_classes", "Number of classes for classification; if "
    "unspecified (or 0), the number of classes found in the labels will be "
    "used.", "c", 0);
PARAM_FLAG("no_intercept", "Do not add the intercept term to the model.", "N");
PARAM_STRING_IN("optimizer", "Optimizer to use for training ('lbfgs' or "
    "'psgd').", "O", "lbfgs");
PARAM_DOUBLE_IN("tolerance", "Convergence tolerance for optimizer.", "e",
    1e-10);
PARAM_INT_IN("max_iterations", "Maximum iterations for optimizer (0 indicates "
    "no limit).", "n", 10000);
PARAM_DOUBLE_IN("step_size", "Step size for parallel SGD optimizer.",
    "s", 0.01);
PARAM_FLAG("shuffle", "Don't shuffle the order in which data points are "
    "visited for parallel SGD.", "S");
PARAM_INT_IN("epochs", "Maximum number of full epochs over dataset for "
    "psgd", "E", 50);
PARAM_INT_IN("seed", "Random seed.  If 0, 'std::time(NULL)' is used.", "r", 0);

// Model loading/saving.
PARAM_MODEL_IN(LinearSVM<>, "input_model", "Existing model "
    "(parameters).", "m");
PARAM_MODEL_OUT(LinearSVM<>, "output_model", "Output for trained "
    "linear svm model.", "M");

// Testing.
PARAM_MATRIX_IN("test", "Matrix containing test dataset.", "T");
PARAM_UROW_IN("test_labels", "Matrix containing test labels.", "A");
PARAM_UROW_OUT("predictions", "If test data is specified, this matrix is where "
    "the predictions for the test set will be saved.", "P");
PARAM_MATRIX_OUT("score", "If test data is specified, this "
    "matrix is where the class score for the test set will be saved.",
    "p");

static void mlpackMain()
{
  if (CLI::GetParam<int>("seed") != 0)
    math::RandomSeed((size_t) CLI::GetParam<int>("seed"));
  else
    math::RandomSeed((size_t) std::time(NULL));

  // Collect command-line options.
  const double lambda = CLI::GetParam<double>("lambda");
  const double delta = CLI::GetParam<double>("delta");
  const string optimizerType = CLI::GetParam<string>("optimizer");
  const double tolerance = CLI::GetParam<double>("tolerance");
  const bool intercept = CLI::HasParam("no_intercept");
  const size_t epochs = (size_t) CLI::GetParam<int>("epochs");
  const size_t maxIterations = (size_t) CLI::GetParam<int>("max_iterations");

  // One of training and input_model must be specified.
  RequireAtLeastOnePassed({ "training", "input_model" }, true);

  // If no output file is given, the user should know that the model will not be
  // saved, but only if a model is being trained.
  RequireAtLeastOnePassed({ "output_model", "predictions", "score"},
      false, "no output will be saved");

  ReportIgnoredParam({{ "test", false }}, "predictions");
  ReportIgnoredParam({{ "test", false }}, "score");
  ReportIgnoredParam({{ "test", false }}, "test_labels");

  // Max Iterations needs to be positive.
  RequireParamValue<int>("max_iterations", [](int x) { return x >= 0; },
      true, "max_iterations must be positive or zero");

  // Tolerance needs to be positive.
  RequireParamValue<double>("tolerance", [](double x) { return x >= 0.0; },
      true, "tolerance must be positive or zero");

  // Optimizer has to be L-BFGS or parallel SGD.
  RequireParamInSet<string>("optimizer", { "lbfgs", "psgd" },
      true, "unknown optimizer");

  // Epochs needs to be non-negative.
  RequireParamValue<int>("epochs", [](int x) { return x >= 0; }, true,
      "maximum number of epochs must be non-negative");

  if (optimizerType != "psgd")
  {
    if (CLI::HasParam("step_size"))
    {
      Log::Warn << PRINT_PARAM_STRING("step_size") << " ignored because "
          << "optimizer type is not 'psgd'." << std::endl;
    }
    if (CLI::HasParam("shuffle"))
    {
      Log::Warn << PRINT_PARAM_STRING("shuffle") << " ignored because "
          << "optimizer type is not 'psgd'." << std::endl;
    }
    if (CLI::HasParam("epochs"))
    {
      Log::Warn << PRINT_PARAM_STRING("epochs") << " ignored because "
          << "optimizer type is not 'psgd'." << std::endl;
    }
  }

  // Step Size must be positive.
  RequireParamValue<double>("step_size", [](double x) { return x >= 0.0; },
      true, "step size must be positive");

  // Lambda must be positive.
  RequireParamValue<double>("lambda", [](double x) { return x >= 0.0; },
      true, "lambda must be positive or zero");

  // Number of Classes must be Non-Negative
  RequireParamValue<int>("number_of_classes", [](int x) { return x >= 0; },
                         true, "number of classes must be greater than or "
                         "equal to 0 (equal to 0 in case of unspecified.)");

  // Delta must be positive.
  RequireParamValue<double>("delta", [](double x) { return x >= 0.0; }, true,
      "delta must be non-negative");

  // Delta must be positive.
  RequireParamValue<int>("epochs", [](int x) { return x > 0; }, true,
      "epochs must be non-negative");

  // These are the matrices we might use.
  arma::mat trainingSet;
  arma::Row<size_t> labels;
  arma::Row<size_t> rawLabels;
  arma::mat testSet;
  arma::Row<size_t> predictions;
  arma::Col<size_t> mappings;
  size_t numClasses;

  // Load data matrix.
  if (CLI::HasParam("training"))
    trainingSet = std::move(CLI::GetParam<arma::mat>("training"));

  // Check if the labels are in a separate file.
  if (CLI::HasParam("training") && CLI::HasParam("labels"))
  {
    rawLabels = std::move(CLI::GetParam<arma::Row<size_t>>("labels"));
    if (trainingSet.n_cols != rawLabels.n_cols)
    {
      Log::Fatal << "The labels must have the same number of points as the "
          << "training dataset." << endl;
    }
  }
  else if (CLI::HasParam("training"))
  {
    // Checking the size of training data if no labels are passed.
    if (trainingSet.n_rows < 2)
    {
      Log::Fatal << "Can't get labels from training data since it has less "
          << "than 2 rows." << endl;
    }

    // The initial predictors for y, Nx1.
    rawLabels = arma::conv_to<arma::Row<size_t>>::from(
        trainingSet.row(trainingSet.n_rows - 1));
    trainingSet.shed_row(trainingSet.n_rows - 1);
  }

  data::NormalizeLabels(rawLabels, labels, mappings);

  // Load the model, if necessary.
  LinearSVM<>* model;
  if (CLI::HasParam("input_model"))
  {
    model = CLI::GetParam<LinearSVM<>*>("input_model");
  }
  else
  {
    numClasses = CLI::GetParam<int>("number_of_classes") == 0 ?
        arma::max(labels) + 1 : CLI::GetParam<int>("number_of_classes");
    model = new LinearSVM<>(0, numClasses);
  }

  // Now, do the training.
  if (CLI::HasParam("training"))
  {
    model->Lambda() = lambda;
    model->Delta() = delta;
    model->NumClasses() = numClasses;

    if (optimizerType == "lbfgs")
    {
      model->FitIntercept() = intercept;

      ens::L_BFGS lbfgsOpt;
      lbfgsOpt.MaxIterations() = maxIterations;
      lbfgsOpt.MinGradientNorm() = tolerance;

      Log::Info << "Training model with L-BFGS optimizer." << endl;

      // This will train the model.
      model->Train(trainingSet, labels, numClasses, lbfgsOpt);
    }
    else if (optimizerType == "psgd")
    {
      const double step_size = CLI::GetParam<double>("step_size");
      const bool shuffle = !CLI::HasParam("shuffle");
      const size_t maxIt = epochs * trainingSet.n_cols;

      ens::ConstantStep decayPolicy(step_size);

      #ifndef HAS_OPENMP
      ens::ParallelSGD<ens::ConstantStep> psgdOpt(maxIt, trainingSet.n_cols,
        tolerance, shuffle, decayPolicy);
      #endif

      // This optimizer is only compiled if OpenMP is used.
      #ifdef HAS_OPENMP
      ens::ParallelSGD<ens::ConstantStep> psgdOpt(maxIt, std::ceil(
        (float)trainingSet.n_cols / omp_get_max_threads()), tolerance, shuffle,
        decayPolicy);
      #endif

      Log::Info << "Training model with ParallelSGD optimizer." << endl;

      // This will train the model.
      model->Train(trainingSet, labels, numClasses, psgdOpt);
    }
  }
  if (CLI::HasParam("test"))
  {
    if (!CLI::HasParam("training"))
    {
      numClasses = model->NumClasses();
    }
    // Get the test dataset, and get predictions.
    testSet = std::move(CLI::GetParam<arma::mat>("test"));
    arma::Row<size_t> predictions;
    size_t trainingDimensionality;

    // Set the dimensionality according to fitintercept.
    if (intercept && optimizerType == "lbfgs")
      trainingDimensionality = model->Parameters().n_rows - 1;
    else
      trainingDimensionality = model->Parameters().n_rows;

    // Checking the dimensionality of the test data.
    if (testSet.n_rows != trainingDimensionality)
    {
      Log::Fatal << "Test data dimensionality (" << testSet.n_rows << ") must "
          << "be the same as the dimensionality of the training data ("
          << trainingDimensionality << ")!" << endl;
    }

    // Save class score, if desired.
    if (CLI::HasParam("score"))
    {
      Log::Info << "Calculating class score of points in '"
          << CLI::GetPrintableParam<arma::mat>("test") << "'." << endl;
      arma::mat score;
      model->Classify(testSet, score);
      CLI::GetParam<arma::mat>("score") = std::move(score);
    }

    model->Classify(testSet, predictions);

    // Calculate accuracy, if desired.
    if (CLI::HasParam("test_labels"))
    {
      arma::Row<size_t> testLabels =
          std::move(CLI::GetParam<arma::Row<size_t>>("test_labels"));

      if (testSet.n_cols != testLabels.n_elem)
      {
        Log::Fatal << "Test data given with " << PRINT_PARAM_STRING("test")
            << " has " << testSet.n_cols << " points, but labels in "
            << PRINT_PARAM_STRING("test_labels") << " have "
            << testLabels.n_elem << " labels!" << endl;
      }

      vector<size_t> correctClassCounts(numClasses, 0);
      vector<size_t> labelSize(numClasses, 0);
      for (arma::uword i = 0; i != predictions.n_elem; ++i)
      {
        if (predictions(i) == testLabels(i))
        {
          ++correctClassCounts[testLabels(i)];
        }
        ++labelSize[testLabels(i)];
      }

      size_t totalCorrectClass = 0;
      for (size_t i = 0; i != correctClassCounts.size(); ++i)
      {
        Log::Info << "Accuracy for points with label " << i << " is "
            << (correctClassCounts[i] / static_cast<double>(labelSize[i]))
            << " (" << correctClassCounts[i] << " of " << labelSize[i] << ")."
            << endl;
        totalCorrectClass += correctClassCounts[i];
      }

      Log::Info << "Total accuracy for all points is "
          << (totalCorrectClass) / static_cast<double>(predictions.n_elem)
          << " (" << totalCorrectClass << " of " << predictions.n_elem << ")."
          << endl;
    }

    // Save predictions, if desired.
    if (CLI::HasParam("predictions"))
    {
      Log::Info << "Predicting classes of points in '"
          << CLI::GetPrintableParam<arma::mat>("test") << "'." << endl;
      CLI::GetParam<arma::Row<size_t>>("predictions") = std::move(predictions);
    }
  }

  CLI::GetParam<LinearSVM<>*>("output_model") = model;
}
