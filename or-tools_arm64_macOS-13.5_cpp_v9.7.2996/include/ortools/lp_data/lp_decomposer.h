// Copyright 2010-2022 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_LP_DATA_LP_DECOMPOSER_H_
#define OR_TOOLS_LP_DATA_LP_DECOMPOSER_H_

#include <memory>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "ortools/lp_data/lp_data.h"
#include "ortools/lp_data/lp_types.h"

namespace operations_research {
namespace glop {

// This class is used to decompose an existing LinearProgram into several
// independent LinearPrograms. Problems are independent when none of their
// variables are connected, i.e. appear in the same constraints.
// Consider for instance the following problem:
//    min: x + 2 y + 3 z + 4 t + 5 u
//    c1: 0 <= x + z <= 1;
//    c2: 0 <= y + t <= 1;
//    c3: 0 <= x + u <= 1;
//    int: x, y, z, t, u
// Variables x, z and u are connected by constraints c1 and c3.
// Variables y and t are connected by constraints c2.
// The problem can be decomposed into two independent problems:
//    min: x + 3 z + 5 u
//    c1: 0 <= x + z <= 1;
//    c3: 0 <= x + u <= 1;
//    int: x, z, u
// and
//    min: 2 y + 4 t
//    c2: 0 <= y + t <= 1;
//    int: y, t
//
// Note that a solution to those two independent problems is a solution to the
// original problem.
class LPDecomposer {
 public:
  LPDecomposer();

  // Decomposes the problem into independent problems.
  // Note that a pointer is kept (no copy) on the linear_problem, so the problem
  // should not change during the life of the LPDecomposer object.
  void Decompose(const LinearProgram* linear_problem)
      ABSL_LOCKS_EXCLUDED(mutex_);

  // Returns the number of independent problems generated by Decompose().
  int GetNumberOfProblems() const ABSL_LOCKS_EXCLUDED(mutex_);

  // Returns the original problem, i.e. as it was before any decomposition.
  const LinearProgram& original_problem() const ABSL_LOCKS_EXCLUDED(mutex_);

  // Fills lp with the problem_index^th independent problem generated by
  // Decompose().
  // Note that this method runs in O(num-entries-in-generated-problem).
  void ExtractLocalProblem(int problem_index, LinearProgram* lp)
      ABSL_LOCKS_EXCLUDED(mutex_);

  // Returns an assignment to the original problem based on the assignments
  // to the independent problems. Requires Decompose() to have been called.
  DenseRow AggregateAssignments(const std::vector<DenseRow>& assignments) const
      ABSL_LOCKS_EXCLUDED(mutex_);

  // Returns an assignment to the given subproblem based on the assignment to
  // the original problem. Requires Decompose() to have been called.
  DenseRow ExtractLocalAssignment(int problem_index, const DenseRow& assignment)
      ABSL_LOCKS_EXCLUDED(mutex_);

 private:
  const LinearProgram* original_problem_;
  std::vector<std::vector<ColIndex>> clusters_;

  mutable absl::Mutex mutex_;

  DISALLOW_COPY_AND_ASSIGN(LPDecomposer);
};

}  // namespace glop
}  // namespace operations_research

#endif  // OR_TOOLS_LP_DATA_LP_DECOMPOSER_H_
