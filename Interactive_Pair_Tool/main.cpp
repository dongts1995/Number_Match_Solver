#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr int kColumns = 9;
    constexpr int kInitialNumbers = 44;
    constexpr int kMaxAdds = 5;

    struct Board
    {
        std::vector<std::vector<int>> cells;
        int appendColumn = 0;
    };

    struct Pair
    {
        int firstRow;
        int firstColumn;
        int secondRow;
        int secondColumn;
        bool diagonal;
        int distance;
        bool removesRow;
    };

    bool occupied(const Board &board, int row, int column)
    {
        return board.cells[row][column] != 0;
    }

    bool inBounds(const Board &board, int row, int column)
    {
        return row >= 0 && row < static_cast<int>(board.cells.size()) &&
               column >= 0 && column < kColumns;
    }

    bool matches(int left, int right)
    {
        return left == right || left + right == 10;
    }

    bool emptyRow(const std::vector<int> &row)
    {
        return std::all_of(row.begin(), row.end(), [](int value)
                           { return value == 0; });
    }

    bool removesRow(const Board &board, const Pair &pair)
    {
        Board next = board;
        next.cells[pair.firstRow][pair.firstColumn] = 0;
        next.cells[pair.secondRow][pair.secondColumn] = 0;
        return std::any_of(next.cells.begin(), next.cells.end(), emptyRow);
    }

    void addPair(std::vector<Pair> &pairs, const Board &board,
                 int firstRow, int firstColumn, int secondRow, int secondColumn)
    {
        const int rowDelta = secondRow - firstRow;
        const int columnDelta = secondColumn - firstColumn;
        Pair pair{firstRow, firstColumn, secondRow, secondColumn,
                  rowDelta != 0 && columnDelta != 0,
                  std::max(std::abs(rowDelta), std::abs(columnDelta)), false};
        pair.removesRow = removesRow(board, pair);
        pairs.push_back(pair);
    }

    std::vector<Pair> availablePairs(const Board &board)
    {
        std::vector<Pair> pairs;
        const int directions[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

        for (int row = 0; row < static_cast<int>(board.cells.size()); ++row)
        {
            for (int column = 0; column < kColumns; ++column)
            {
                if (!occupied(board, row, column))
                    continue;

                for (const auto &direction : directions)
                {
                    int nextRow = row + direction[0];
                    int nextColumn = column + direction[1];
                    while (inBounds(board, nextRow, nextColumn) &&
                           !occupied(board, nextRow, nextColumn))
                    {
                        nextRow += direction[0];
                        nextColumn += direction[1];
                    }
                    if (inBounds(board, nextRow, nextColumn) &&
                        (nextRow > row || (nextRow == row && nextColumn > column)) &&
                        matches(board.cells[row][column], board.cells[nextRow][nextColumn]))
                    {
                        addPair(pairs, board, row, column, nextRow, nextColumn);
                    }
                }
            }
        }

        for (int row = 0; row + 1 < static_cast<int>(board.cells.size()); ++row)
        {
            int last = -1;
            int first = -1;
            for (int column = kColumns - 1; column >= 0; --column)
            {
                if (occupied(board, row, column))
                {
                    last = column;
                    break;
                }
            }
            for (int column = 0; column < kColumns; ++column)
            {
                if (occupied(board, row + 1, column))
                {
                    first = column;
                    break;
                }
            }
            if (last >= 0 && first >= 0 &&
                matches(board.cells[row][last], board.cells[row + 1][first]))
            {
                addPair(pairs, board, row, last, row + 1, first);
            }
        }

        std::sort(pairs.begin(), pairs.end(), [](const Pair &left, const Pair &right)
                  {
        if (left.diagonal != right.diagonal)
            return left.diagonal > right.diagonal;
        if (left.distance != right.distance)
            return left.distance > right.distance;
        if (left.removesRow != right.removesRow)
            return left.removesRow < right.removesRow;
        if (left.firstRow != right.firstRow)
            return left.firstRow < right.firstRow;
        if (left.firstColumn != right.firstColumn)
            return left.firstColumn < right.firstColumn;
        if (left.secondRow != right.secondRow)
            return left.secondRow < right.secondRow;
        return left.secondColumn < right.secondColumn; });
        return pairs;
    }

    void removePair(Board &board, const Pair &pair)
    {
        const bool lastRowWasPartial = board.appendColumn != 0;
        board.cells[pair.firstRow][pair.firstColumn] = 0;
        board.cells[pair.secondRow][pair.secondColumn] = 0;
        const bool lastRowWasEmpty = emptyRow(board.cells.back());
        board.cells.erase(std::remove_if(board.cells.begin(), board.cells.end(), emptyRow),
                          board.cells.end());
        if (board.cells.empty())
            board.appendColumn = 0;
        else if (lastRowWasPartial && lastRowWasEmpty)
            board.appendColumn = 0;
    }

    void addRemainingNumbers(Board &board)
    {
        std::vector<int> remaining;
        for (const auto &row : board.cells)
        {
            for (int value : row)
            {
                if (value != 0)
                    remaining.push_back(value);
            }
        }

        if (remaining.empty())
            return;

        size_t index = 0;
        if (board.appendColumn != 0)
        {
            const int available = kColumns - board.appendColumn;
            const int count = std::min(available, static_cast<int>(remaining.size()));
            for (int offset = 0; offset < count; ++offset)
            {
                board.cells.back()[board.appendColumn + offset] = remaining[index++];
            }
            board.appendColumn += count;
            if (board.appendColumn == kColumns)
                board.appendColumn = 0;
        }

        while (index < remaining.size())
        {
            board.cells.push_back(std::vector<int>(kColumns, 0));
            const int count = std::min(kColumns, static_cast<int>(remaining.size() - index));
            for (int column = 0; column < count; ++column)
                board.cells.back()[column] = remaining[index++];
            board.appendColumn = count < kColumns ? count : 0;
        }
    }

    void printBoard(const Board &board)
    {
        std::cout << "\nMa tran hien tai ('.' = o xam):\n    ";
        for (int column = 1; column <= kColumns; ++column)
            std::cout << column << ' ';
        std::cout << '\n';
        for (int row = 0; row < static_cast<int>(board.cells.size()); ++row)
        {
            std::cout << row + 1 << " | ";
            for (int column = 0; column < kColumns; ++column)
            {
                const int value = board.cells[row][column];
                const bool notAddedYet = row == static_cast<int>(board.cells.size()) - 1 &&
                                         board.appendColumn != 0 &&
                                         column >= board.appendColumn;
                std::cout << (notAddedYet ? ' ' : (value == 0 ? '.' : static_cast<char>('0' + value)))
                          << ' ';
            }
            std::cout << '\n';
        }
    }

    void printPairs(const Board &board, const std::vector<Pair> &pairs)
    {
        std::cout << "\nCac cap available (uu tien cheo, xa, khong xoa hang):\n";
        for (size_t index = 0; index < pairs.size(); ++index)
        {
            const Pair &pair = pairs[index];
            std::cout << index + 1 << ". (" << pair.firstRow + 1 << ',' << pair.firstColumn + 1
                      << ")=" << board.cells[pair.firstRow][pair.firstColumn]
                      << " <-> (" << pair.secondRow + 1 << ',' << pair.secondColumn + 1
                      << ")=" << board.cells[pair.secondRow][pair.secondColumn]
                      << (pair.diagonal ? " [cheo" : " [thang/dung")
                      << ", cach " << pair.distance
                      << (pair.removesRow ? ", xoa hang]" : ", giu hang]") << '\n';
        }
    }

    bool parseInput(const std::string &input, Board &board, std::vector<int> &values)
    {
        for (char character : input)
        {
            if (character == ' ' || character == '\t' || character == '\r' || character == '\n')
                continue;
            if (character < '1' || character > '9')
                return false;
            values.push_back(character - '0');
        }
        if (values.empty())
            return false;

        const size_t initialCount = std::min(values.size(), static_cast<size_t>(kInitialNumbers));
        for (size_t index = 0; index < initialCount; ++index)
        {
            if (index % kColumns == 0)
                board.cells.push_back(std::vector<int>(kColumns, 0));
            board.cells.back()[index % kColumns] = values[index];
        }
        board.appendColumn = static_cast<int>(initialCount % kColumns);
        return true;
    }
}

int main(int argc, char *argv[])
{
    std::string input;
    if (argc >= 2)
        input = argv[1];
    else
        std::getline(std::cin, input);

    Board board;
    std::vector<int> values;
    if (!parseInput(input, board, values))
    {
        std::cerr << "Loi: input phai la chuoi cac so tu 1 den 9.\n";
        return 1;
    }

    int addCount = 0;
    while (!board.cells.empty())
    {
        printBoard(board);
        const std::vector<Pair> pairs = availablePairs(board);
        if (pairs.empty())
        {
            std::cout << "\nKhong con cap available.";
        }
        else
        {
            printPairs(board, pairs);
        }
        std::cout << "\nChon cap";
        if (!pairs.empty())
            std::cout << " (1-" << pairs.size() << ")";
        std::cout << ", + (" << addCount << "/" << kMaxAdds << ") hoac q: ";

        std::string choice;
        if (!std::getline(std::cin, choice))
            break;
        if (choice == "q" || choice == "Q")
            break;
        if (choice == "+")
        {
            if (addCount >= kMaxAdds)
            {
                std::cout << "Da dat gioi han 5 lan bam +.\n";
                continue;
            }
            addRemainingNumbers(board);
            ++addCount;
            std::cout << "Da bam + (" << addCount << "/" << kMaxAdds << ").\n";
            continue;
        }

        std::istringstream parser(choice);
        int selected = 0;
        char extra = 0;
        if (!(parser >> selected) || (parser >> extra) ||
            pairs.empty() || selected < 1 || selected > static_cast<int>(pairs.size()))
        {
            std::cout << "Lua chon khong hop le.\n";
            continue;
        }

        removePair(board, pairs[selected - 1]);
        if (board.cells.empty())
        {
            std::cout << "\nChuc mung, da xoa sach ma tran.\n";
            break;
        }
    }
    return 0;
}