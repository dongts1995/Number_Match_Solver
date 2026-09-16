#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{

    constexpr int kColumns = 9;
    constexpr int kInitialNumbers = 44;
    constexpr int kMaxAdds = 5;

    struct Board
    {
        // 0 = empty / grey
        // 1..9 = number
        std::vector<std::vector<int>> cells;

        // Vị trí tiếp theo sẽ được append vào hàng cuối.
        //
        // Ví dụ:
        // appendColumn = 8
        //
        // nghĩa là hàng cuối đang có số ở column 0..7
        // và số tiếp theo sẽ vào column 8.
        //
        // appendColumn = 0
        // nghĩa là hàng cuối đã full 9 ô,
        // số tiếp theo phải tạo hàng mới.
        int appendColumn = 0;

        bool empty() const
        {
            return cells.empty();
        }
    };

    struct Move
    {
        int firstRow = 0;
        int firstColumn = 0;

        int secondRow = 0;
        int secondColumn = 0;

        bool diagonal = false;
        int distance = 0;

        bool removesRow = false;
    };

    struct Solution
    {
        bool found = false;

        int steps = std::numeric_limits<int>::max();
        int adds = std::numeric_limits<int>::max();

        std::vector<std::string> actions;
    };

    struct Solver
    {
        std::vector<int> sequence;

        Solution best;

        std::unordered_map<std::string, int> memo;

        // ============================================================
        // Basic helpers
        // ============================================================

        static bool matches(int left, int right)
        {
            return left == right || left + right == 10;
        }

        static bool occupied(
            const Board &board,
            int row,
            int column)
        {
            return board.cells[row][column] != 0;
        }

        static bool isDiagonal(
            int rowDelta,
            int columnDelta)
        {
            return rowDelta != 0 && columnDelta != 0;
        }

        static bool rowIsEmpty(
            const std::vector<int> &row)
        {
            return std::all_of(
                row.begin(),
                row.end(),
                [](int value)
                {
                    return value == 0;
                });
        }

        static bool inBounds(
            const Board &board,
            int row,
            int column)
        {
            return row >= 0 &&
                   row < static_cast<int>(board.cells.size()) &&
                   column >= 0 &&
                   column < kColumns;
        }

        // ============================================================
        // Remove completely empty rows
        // ============================================================

        static void removeEmptyRows(Board &board)
        {
            if (board.cells.empty())
            {
                board.appendColumn = 0;
                return;
            }

            /*
             * Nếu hàng cuối hiện tại là partial row và bị xóa hoàn toàn,
             * thì sau khi xóa, hàng cuối mới có thể là:
             *
             * 1. full row  -> appendColumn = 0
             * 2. partial   -> cần tính lại appendColumn
             *
             * Tuy nhiên trong game, chỉ hàng cuối cùng mới có thể là
             * partial row. Các hàng phía trên luôn full 9 ô về mặt
             * cấu trúc append.
             */

            const bool lastRowWasPartial =
                board.appendColumn != 0;
            const bool lastRowWasEmpty =
                rowIsEmpty(board.cells.back());

            board.cells.erase(
                std::remove_if(
                    board.cells.begin(),
                    board.cells.end(),
                    rowIsEmpty),
                board.cells.end());

            if (board.cells.empty())
            {
                board.appendColumn = 0;
                return;
            }

            /*
             * Nếu hàng cuối cũ là partial và đã bị remove,
             * hàng cuối mới phải trở thành full row.
             *
             * Ví dụ:
             *
             * row 1: full
             * row 2: full
             * row 3: partial -> bị xóa
             *
             * Sau khi xóa:
             *
             * row 1: full
             * row 2: full
             *
             * => appendColumn = 0
             */
            if (lastRowWasPartial && lastRowWasEmpty)
            {
                board.appendColumn = 0;
            }
        }

        // ============================================================
        // Check whether this move removes a complete row
        // ============================================================

        static bool wouldRemoveRow(
            const Board &board,
            const Move &move)
        {
            Board next = board;

            next.cells[move.firstRow][move.firstColumn] = 0;
            next.cells[move.secondRow][move.secondColumn] = 0;

            return std::any_of(
                next.cells.begin(),
                next.cells.end(),
                rowIsEmpty);
        }

        // ============================================================
        // Add move
        // ============================================================

        static void addMove(
            std::vector<Move> &moves,
            const Board &board,
            int firstRow,
            int firstColumn,
            int secondRow,
            int secondColumn)
        {
            const int rowDelta =
                secondRow - firstRow;

            const int columnDelta =
                secondColumn - firstColumn;

            Move move;

            move.firstRow = firstRow;
            move.firstColumn = firstColumn;

            move.secondRow = secondRow;
            move.secondColumn = secondColumn;

            move.diagonal =
                isDiagonal(rowDelta, columnDelta);

            move.distance =
                std::max(
                    std::abs(rowDelta),
                    std::abs(columnDelta));

            move.removesRow =
                wouldRemoveRow(board, move);

            moves.push_back(move);
        }

        // ============================================================
        // Find all legal moves
        // ============================================================

        static std::vector<Move> legalMoves(
            const Board &board)
        {
            std::vector<Move> moves;

            /*
             * 8 directions:
             *
             * NW N NE
             * W     E
             * SW S SE
             */
            const int directions[8][2] =
                {
                    {-1, -1},
                    {-1, 0},
                    {-1, 1},
                    {0, -1},
                    {0, 1},
                    {1, -1},
                    {1, 0},
                    {1, 1}};

            // ========================================================
            // Normal horizontal / vertical / diagonal moves
            // ========================================================

            for (int row = 0;
                 row < static_cast<int>(board.cells.size());
                 ++row)
            {
                for (int column = 0;
                     column < kColumns;
                     ++column)
                {
                    if (!occupied(board, row, column))
                    {
                        continue;
                    }

                    for (const auto &direction : directions)
                    {
                        int nextRow =
                            row + direction[0];

                        int nextColumn =
                            column + direction[1];

                        /*
                         * Skip all grey cells.
                         */
                        while (
                            inBounds(
                                board,
                                nextRow,
                                nextColumn) &&
                            !occupied(
                                board,
                                nextRow,
                                nextColumn))
                        {
                            nextRow += direction[0];
                            nextColumn += direction[1];
                        }

                        /*
                         * Found another number.
                         */
                        if (
                            inBounds(
                                board,
                                nextRow,
                                nextColumn) &&
                            (nextRow > row ||
                             (nextRow == row &&
                              nextColumn > column)) &&
                            matches(
                                board.cells[row][column],
                                board.cells[nextRow][nextColumn]))
                        {
                            addMove(
                                moves,
                                board,
                                row,
                                column,
                                nextRow,
                                nextColumn);
                        }
                    }
                }
            }

            // ========================================================
            // Special connection:
            //
            // Last remaining number of row N
            //              +
            // First remaining number of row N+1
            //
            // ========================================================

            for (
                int row = 0;
                row + 1 < static_cast<int>(board.cells.size());
                ++row)
            {
                int lastColumn = -1;

                /*
                 * Find last occupied cell in current row.
                 */
                for (int column = kColumns - 1;
                     column >= 0;
                     --column)
                {
                    if (occupied(board, row, column))
                    {
                        lastColumn = column;
                        break;
                    }
                }

                int firstColumn = -1;

                /*
                 * Find first occupied cell in next row.
                 */
                for (int column = 0;
                     column < kColumns;
                     ++column)
                {
                    if (occupied(board, row + 1, column))
                    {
                        firstColumn = column;
                        break;
                    }
                }

                if (
                    lastColumn >= 0 &&
                    firstColumn >= 0 &&
                    matches(
                        board.cells[row][lastColumn],
                        board.cells[row + 1][firstColumn]))
                {
                    addMove(
                        moves,
                        board,
                        row,
                        lastColumn,
                        row + 1,
                        firstColumn);
                }
            }

            // ========================================================
            // Sort moves
            // ========================================================

            std::sort(
                moves.begin(),
                moves.end(),
                [](const Move &left, const Move &right)
                {
                    if (left.diagonal != right.diagonal)
                    {
                        return left.diagonal > right.diagonal;
                    }

                    if (left.distance != right.distance)
                    {
                        return left.distance > right.distance;
                    }

                    if (left.removesRow != right.removesRow)
                    {
                        return left.removesRow < right.removesRow;
                    }

                    if (left.firstRow != right.firstRow)
                    {
                        return left.firstRow < right.firstRow;
                    }

                    if (left.firstColumn != right.firstColumn)
                    {
                        return left.firstColumn < right.firstColumn;
                    }

                    if (left.secondRow != right.secondRow)
                    {
                        return left.secondRow < right.secondRow;
                    }

                    return left.secondColumn <
                           right.secondColumn;
                });

            return moves;
        }

        // ============================================================
        // Apply move
        // ============================================================

        static Board applyMove(
            Board board,
            const Move &move)
        {
            board.cells
                [move.firstRow]
                [move.firstColumn] = 0;

            board.cells
                [move.secondRow]
                [move.secondColumn] = 0;

            removeEmptyRows(board);

            return board;
        }

        // ============================================================
        // Append numbers after pressing '+'
        // ============================================================

        static void appendNumbers(
            Board &board,
            const std::vector<int> &numbers)
        {
            if (numbers.empty())
            {
                return;
            }

            std::size_t index = 0;

            // ========================================================
            // Case 1:
            // Current last row is partial.
            //
            // Example:
            //
            // 2 7 . . . . . . .
            //
            // appendColumn = 2
            //
            // first number goes to column 3.
            // ========================================================

            if (!board.cells.empty() &&
                board.appendColumn != 0)
            {
                const int available =
                    kColumns - board.appendColumn;

                const int count =
                    std::min(
                        available,
                        static_cast<int>(
                            numbers.size()));

                for (int i = 0;
                     i < count;
                     ++i)
                {
                    board.cells.back()
                        [board.appendColumn + i] =
                        numbers[index++];
                }

                board.appendColumn += count;

                /*
                 * Last row became full.
                 *
                 * Next append must create a new row.
                 */
                if (board.appendColumn == kColumns)
                {
                    board.appendColumn = 0;
                }
            }

            // ========================================================
            // Case 2:
            // Need new rows.
            // ========================================================

            while (index < numbers.size())
            {
                board.cells.push_back(
                    std::vector<int>(
                        kColumns,
                        0));

                const int count =
                    std::min(
                        kColumns,
                        static_cast<int>(
                            numbers.size() - index));

                for (int column = 0;
                     column < count;
                     ++column)
                {
                    board.cells.back()[column] =
                        numbers[index++];
                }

                /*
                 * If this row is partial, remember the next
                 * append position.
                 *
                 * If it is full, appendColumn = 0.
                 */
                if (count < kColumns)
                {
                    board.appendColumn = count;
                }
                else
                {
                    board.appendColumn = 0;
                }
            }
        }

        // ============================================================
        // Encode board for memoization
        // ============================================================

        static std::string encode(
            const Board &board,
            int adds,
            int nextNumberIndex)
        {
            std::string key =
                std::to_string(board.appendColumn) +
                ":" +
                std::to_string(adds) +
                ":" +
                std::to_string(nextNumberIndex) +
                ":";

            for (const auto &row : board.cells)
            {
                for (int value : row)
                {
                    key +=
                        static_cast<char>(
                            '0' + value);
                }

                key += '/';
            }

            return key;
        }

        // ============================================================
        // Describe move
        // ============================================================

        static std::string describe(
            const Move &move,
            int firstValue,
            int secondValue)
        {
            std::ostringstream output;

            output
                << "Xoa ("
                << move.firstRow + 1
                << ","
                << move.firstColumn + 1
                << ")="
                << firstValue
                << " va ("
                << move.secondRow + 1
                << ","
                << move.secondColumn + 1
                << ")="
                << secondValue;

            output
                << (move.diagonal
                        ? " [cheo"
                        : " [thang/dung");

            output
                << ", cach "
                << move.distance;

            output
                << (move.removesRow
                        ? ", day hang]"
                        : ", khong day hang]");

            return output.str();
        }

        // ============================================================
        // Search
        // ============================================================

        void search(
            const Board &board,
            int adds,
            int removeSteps,
            int nextNumberIndex,
            std::vector<std::string> &actions)
        {
            // ========================================================
            // Win
            // ========================================================

            if (board.empty())
            {
                best.found = true;
                best.steps = removeSteps;
                best.adds = adds;
                best.actions = actions;

                return;
            }

            // ========================================================
            // Maximum '+'
            // ========================================================

            if (adds > kMaxAdds)
            {
                return;
            }

            // ========================================================
            // Memoization
            // ========================================================

            const std::string key =
                encode(board, adds, nextNumberIndex);

            auto memoIt =
                memo.find(key);

            if (
                memoIt != memo.end() &&
                memoIt->second <= removeSteps)
            {
                return;
            }

            memo[key] = removeSteps;

            // ========================================================
            // Find legal moves
            // ========================================================

            const auto moves =
                legalMoves(board);

            // ========================================================
            // Try removing numbers
            // ========================================================

            for (const Move &move : moves)
            {
                actions.push_back(
                    describe(
                        move,
                        board.cells
                            [move.firstRow]
                            [move.firstColumn],
                        board.cells
                            [move.secondRow]
                            [move.secondColumn]));

                const Board next =
                    applyMove(
                        board,
                        move);

                search(
                    next,
                    adds,
                    removeSteps + 1,
                    nextNumberIndex,
                    actions);

                actions.pop_back();

                /*
                 * Stop immediately when a solution
                 * has been found.
                 */
                if (best.found)
                {
                    return;
                }
            }

            // ========================================================
            // No move available -> press '+'
            // ========================================================

            if (
                moves.empty() &&
                adds < kMaxAdds &&
                nextNumberIndex < static_cast<int>(sequence.size()))
            {
                const std::vector<int> remaining(
                    sequence.begin() + nextNumberIndex,
                    sequence.end());

                if (!remaining.empty())
                {
                    Board expanded = board;

                    /*
                     * IMPORTANT:
                     *
                     * Append exactly the remaining numbers
                     * in row-major order.
                     */
                    appendNumbers(
                        expanded,
                        remaining);

                    actions.push_back(
                        "Bam +: them "
                        "so con lai vao cuoi ma tran");

                    search(
                        expanded,
                        adds + 1,
                        removeSteps,
                        nextNumberIndex + static_cast<int>(remaining.size()),
                        actions);

                    actions.pop_back();
                }
            }
        }

        // ============================================================
        // Solve
        // ============================================================

        Solution solve()
        {
            Board initial;

            int index = 0;

            /*
             * Initial 44 numbers:
             *
             * 9
             * 9
             * 9
             * 9
             * 8
             *
             * => 5 rows
             */
            while (
                index < static_cast<int>(
                            sequence.size()) &&
                index < kInitialNumbers)
            {
                if (index % kColumns == 0)
                {
                    initial.cells.push_back(
                        std::vector<int>(
                            kColumns,
                            0));
                }

                initial.cells.back()
                    [index % kColumns] =
                    sequence[index++];
            }

            if (initial.empty())
            {
                return best;
            }

            /*
             * 44 % 9 = 8
             *
             * Therefore:
             *
             * row 1: 9
             * row 2: 9
             * row 3: 9
             * row 4: 9
             * row 5: 8
             *
             * The next number goes to column 9
             * of row 5.
             */
            initial.appendColumn =
                index % kColumns;

            search(
                initial,
                0,
                0,
                index,
                std::vector<std::string>{});

            return best;
        }
    };

    // ================================================================
    // Input parser
    // ================================================================

    bool parseInput(
        const std::string &text,
        std::vector<int> &numbers,
        std::string &error)
    {
        for (char character : text)
        {
            if (
                character == ' ' ||
                character == '\t' ||
                character == '\r' ||
                character == '\n')
            {
                continue;
            }

            if (
                character < '1' ||
                character > '9')
            {
                error =
                    "Input chi duoc chua "
                    "cac chu so tu 1 den 9.";

                return false;
            }

            numbers.push_back(
                character - '0');
        }

        if (numbers.empty())
        {
            error =
                "Input khong duoc rong.";

            return false;
        }

        return true;
    }

} // namespace

// ================================================================
// MAIN
// ================================================================

int main(
    int argc,
    char *argv[])
{
    std::string input;

    if (argc >= 2)
    {
        input = argv[1];
    }
    else
    {
        std::getline(
            std::cin,
            input);
    }

    // ============================================================
    // Parse input
    // ============================================================

    std::vector<int> numbers;

    std::string error;

    if (
        !parseInput(
            input,
            numbers,
            error))
    {
        std::cerr
            << "Loi: "
            << error
            << '\n';

        return 1;
    }

    // ============================================================
    // Solve
    // ============================================================

    Solver solver;

    solver.sequence =
        std::move(numbers);

    const Solution solution =
        solver.solve();

    // ============================================================
    // No solution
    // ============================================================

    if (!solution.found)
    {
        std::cout
            << "Khong tim thay solution "
            << "trong gioi han "
            << kMaxAdds
            << " lan bam +.\n";

        return 2;
    }

    // ============================================================
    // Print solution
    // ============================================================

    std::cout
        << "Solution: "
        << solution.steps
        << " buoc xoa, "
        << solution.adds
        << " lan bam +\n";

    for (
        std::size_t index = 0;
        index < solution.actions.size();
        ++index)
    {
        std::cout
            << index + 1
            << ". "
            << solution.actions[index]
            << '\n';
    }

    return 0;
}
