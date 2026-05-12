/**
 * @file Web.h
 * @brief Live range and web (merged live range) representation.
 *
 * A live range of a variable is a list of program lines on which that variable
 * is live, with the first line optionally annotated with @c '+' (the line where
 * the range is born, i.e. an LHS write) and/or the last line annotated with
 * @c '-' (the last use, after which the value is dead).
 *
 * A @ref Web is the union of live ranges of the same variable that overlap on
 * at least one program point. Webs are the actual unit of register allocation
 * — each web competes for a single register (or memory) over its entire span.
 */
#pragma once

#include <string>
#include <vector>

/**
 * @brief Single program point inside a live range or web.
 *
 * @c marker is one of:
 *   - @c '+' — the line defines the variable (LHS write, range start);
 *   - @c '-' — the line is the last use (RHS read, range end);
 *   - @c ' ' — the line is an internal/flow-through point.
 */
struct ProgramPoint {
    int line;     ///< program line number
    char marker;  ///< '+' , '-' , or ' '
};

/**
 * @brief A parsed live range.
 *
 * @c variable holds the symbolic name. @c points is ordered as parsed
 * from the input file (input format already lists points in execution order).
 */
struct LiveRange {
    std::string variable;
    std::vector<ProgramPoint> points;
};

/**
 * @brief A web — the unit of register allocation.
 *
 * Built from one or more @ref LiveRange instances of the same variable that
 * share at least one program point.
 */
class Web {
public:
    int id = -1;                     ///< web index (web0, web1, ...) assigned on output
    std::string variable;            ///< source variable name
    std::vector<ProgramPoint> points;///< program points sorted ascending by line

    /**
     * @brief Merge two webs (same variable) by union of program points.
     *
     * If a line has @c '+' in one web and @c '-' in the other (the "i = i + 1"
     * pattern) the two markers cancel out and the merged point becomes an
     * internal flow-through point.
     *
     * @complexity O((|A| + |B|) · log(|A| + |B|))
     */
    static Web merge(const Web &a, const Web &b);

    /**
     * @brief Test whether two webs (same variable) share at least one program line.
     * @complexity O(|A| · |B|)
     */
    static bool overlap(const Web &a, const Web &b);

    /**
     * @brief True iff the web contains program line @p line.
     * @complexity O(|points|)
     */
    bool containsLine(int line) const;

    /**
     * @brief Marker stored at line @p line.
     * @return '+' , '-' , ' ' (line present, no marker), or '?' if absent.
     */
    char markerAt(int line) const;

    /**
     * @brief Interference test for the interference graph construction.
     *
     * Two webs interfere iff they share at least one program line on which
     * @b both are simultaneously live. Per the spec, a single shared line where
     * one web has @c '+' (definition starts here) and the other has @c '-'
     * (last use here) does @b not count as an interfering point, because the
     * value of the dying web is no longer needed once the new one is defined.
     *
     * @complexity O(|A| + |B|) — both point lists are kept sorted.
     */
    bool interferesWith(const Web &other) const;

    /**
     * @brief Human-readable line list in the spec's output format
     *        (e.g. "1+,2,3,4,5,6-").
     */
    std::string serialize() const;
};

/**
 * @brief Group ranges of the same variable that share program lines.
 *
 * Implements a simple union-find pass: for each variable, partitions its
 * ranges into connected components where two ranges are connected iff they
 * share any program line. Each component becomes a web.
 *
 * The returned webs are sorted alphabetically by variable name and, within
 * the same variable, by smallest contained line (so output is deterministic
 * and matches the spec's example).
 *
 * @complexity O(R² · L) worst case (R = #ranges, L = max range length).
 */
std::vector<Web> buildWebs(const std::vector<LiveRange> &ranges);
