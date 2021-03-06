"""
Let N be the number of intervals in a trigger set. For example,
      For a trigger set '[0:4) [10:12)' N would be '2'.

Let E be the extend of a trigger set, i.e. the number of code points 
      that it contains. The trigger set '[0:4) [10:12)' has an 
      extend of '12', i.e. the numbers from 0 to 11 (12 is excluded).

Let C be the computation cost for an operation.
 
Let M be the memory cost for an operation.

(i) Cost of Bisectioning _______________________________________________

    Example:

        if x > 64:
            if x < 16:
                if x == 4: jump state1
            else:
                jump state2
        else:
            jump state2

    C = log2(N) * (cmp + jump)
    M = log2(N) * (cmp + jump)

(ii) Branch Tables / Switch Statements _________________________________

     Branch tables are generated by the compiler from 'switch' statements
     with 'x' containing the value to branch upon:

         (1) x = validate x in range   # x = 0 if not in range
         (2) i = x * sizeof(goto)
         (3) jump $(next + i)          # goto address in 'next[i]'

        next: bad_state
              state_1
              state_2
              ...

     Thus, the resulting cost is:

          (1) Two comparisons, if 'x' does not cover a byte range.
              (this includes a conditional jump)
          (2) shift operation (times '4' = << 2, for example)
          (3) addition + referencing + jump

     C = 1 * (2 * cmp + jump + shift + add + ref + jump)
     M = E * (2 * cmp + jump + shift + add + ref + jump)

     where 'C' is indenpendent of 'E' and 'N', but M linear with E.



HEURISTIC: _____________________________________________________________

   (1) If interval can be interpreted as set of 'exceptional' transitions
       where the number of exceptions N <= N_linear, the implement trigger
       map as Linear Comparison (iii).

   (2) 
"""
from   quex.engine.misc.enum import Enum
from   quex.blackboard       import setup as Setup
from   math                  import log

E_Type = Enum("SWITCH_CASE", "BISECTION", "COMPARISON_SEQUENCE", "TRANSITION")

#class Cost:
#    def __init__(self, C, M):
#        self.computation = C
#        self.memory      = M
#
class Region:
    @classmethod
    def init(cls):
        del cls.candidate_list[:]
        del cls.good_list[:]

    @classmethod
    def consider(cls, EntryIndex, TheInterval, TheTarget):
        cls.generate_candidates(cls.candidate_list, EntryIndex, TheInterval, TheTarget)

    @classmethod
    def terminate(cls, TheInterval, TheTarget):
        i    = 0
        size = len(cls.candidate_list)
        while i < size:
            candidate = cls.candidate_list
            if candidate.absorb(TheInterval, Target): 
                i += 1
            else:
                # Candidate has not absorbed the interval. So, its finished.
                # If it is good, then take it. Otherwise forget about it.
                if candidate.good():
                    cls.good_list.append(candidate)
                del candidate_list[i]
                size -= 1

     @staticmethod
     def elect(RegionList*):
         while 1 + 1 == 2:
             # previous.begin <= current.begin
             if current.begin < previous.end:
                 if current.value() > previous.value(): delete(previous)
                 else:                                  delete(current)

     @property
     def size(self):
         return self.end_i - self.begin_i

class LinearComparisonRegion(Region):
    """Linear Comparison (forward/backward) _____________________________

          if x == value1: goto state_1      'specific'
          if x == value2: goto state_2      'specific'
          ...
          else:           goto state_else   'else'

       Costs: Computation = N / 2 * (cmp + jump)    (in average)
              Memory      = N * (cmp + jump)

       where 'N' is the number of values in the given interval where the target
       differs from 'state_else'.
    """
    candidate_list = []
    good_list      = []

    def __init__(self, BeginIndex, TheInterval, TheTarget, ElseF):
        self.begin_i = BeginIndex
        self.end_i   = BeginIndex + 1 # point after the last element
        if not ElseF:
            for i in xrange(TheInterval.begin, TheInterval.end):
                self.specific_list.append((i, TheTarget))
            self.else_target = None
            self.last_else_f = False
        else:
            self.else_target = TheTarget 
            self.last_else_f = True

    @staticmethod
    def generate_candidates(candidate_list, EntryIndex, TheInterval, TheTarget):
        candidate_list.append(
            LinearComparisonRegion(EntryIndex, TheInterval, TheTarget, ElseF=True) 
        )
        if len(TheInterval) < LinearComparisonRegion.max_n:
            candidate_list.append(
                LinearComparisonRegion(EntryIndex, TheInterval, TheTarget, ElseF=False)
            )

    def good(self):
        return len(self.specific_list) > LinearComparisonRegion.min_n

    def absorb(self, TheInterval, TheTarget):
        if not self.last_else_f:
            # Last target was specific => current may be 'else' or 'specific'
            if TheTarget == self.else_target:
                self.end_i += 1
                self.last_else_f = True
                return True
        else:
            pass # Last target was 'else' => current must be 'specific'

        if TheInterval.size() + len(self.triggerdb) > LinearComparisonRegion.max_n:
            return False

        for i in xrange(TheInterval.begin, TheInterval.end):
            self.specific_list.append((i, TheTarget))
        self.end_i += 1
        self.last_else_f = False

        return True







def watch(TriggerMap):
    LinearComparison.init()
    BranchTable.init()

    for i, info in enumerate(TriggerMap):
        interval, target = info

        LinearComparison.consider(i, interval, target)
        LinearComparison.terminate(interval, target)

        BranchTable.consider(i, interval, target)
        BranchTable.terminate(interval, target)

    good_list = Region.elect(LinearComparison.good_list, 
                             BranchTable.good_list)

    implant(TriggerMap, good_list)

def implant(trigger_map, RegionList):
    offset = 0
    for region in RegionList:
        insertion_i = region.begin_i - offset
        trigger_map[insertion_i] = region
        del trigger_map[insertion_i + 1: insertion_i + region.size]
        offset += region.size


def get(TriggerMap, 
        size_all_intervals=None, 
        size_all_drop_out_intervals=None):
    TriggerSetN       = len(TriggerMap)

    if TriggerSetN == 1:
        return E_Type.TRANSITION

    if size_all_intervals is None:
        size_all_intervals          = 0
        size_all_drop_out_intervals = 0
        for interval, target in TriggerMap:
            if target.drop_out_f: size_all_drop_out_intervals += interval.size()
            size_all_intervals += interval.size()

    if size_all_intervals - size_all_drop_out_intervals == 0:
        # No drop-outs
        return E_Type.BISECTION

    p = log(len(TriggerMap), 2) / (size_all_intervals - size_all_drop_out_intervals)

    if p > 0.03: 
        return E_Type.SWITCH_CASE

    if len(TriggerMap) > 5: 
        return E_Type.BISECTION
    else:
        return E_Type.COMPARISON_SEQUENCE

def prune_range(TriggerMap):
    """Consider the 'useful range' starting from zero. Thus, the first 
       interval to be considered is the first that intersects with 0.
       Then 'begin' must become '0' instead of a negative value.
    """
    LowerLimit = 0
    UpperLimit = Setup.get_character_value_limit()

    # Iterate from 'low' to 'high'
    for i, info in enumerate(TriggerMap):
        interval, target = info
        if interval.end <= LowerLimit: continue

        # Found an interval that intersects with 'LowerLimit' line
        interval.begin = LowerLimit
        if i != 0: del TriggerMap[:i]
        return

    L = len(TriggerMap)
    # Iterate from 'high' to 'low'
    for ii, info in enumerate(reversed(TriggerMap)):
        interval, target = info
        if interval.begin > UpperLimit: continue

        # Found an interval that intersects with 'UpperLimit' line
        interval.end = UpperLimit 
        if ii != 0: 
            i = L - ii - 1  # index = Size - inverse index - 1
            del TriggerMap[i+1:]
        return

    # The whole trigger map happens below 0. This is trash, no doubt!
    assert False

def prune_outstanding(TriggerMap):
    """Implements the remaining transitions as:

       (1) Check for an exceptionally often character
       (2) Check for the remaining trigger map
    """
    # Currently no outstanding characters are determined (no statistics yet)
    return None

    #assert TriggerMap[EntryIndex].size() == 1
    #OutstandingCharacter = TriggerMap[EntryIndex].begin

    #if EntryIndex != 0 and EntryIndex != len(TriggerMap) - 1:
    #    # Leave the entry before at size '1' because its easier to test
    #    if   TriggerMap[EntryIndex-1].size() == 1: TriggerMap[EntryIndex+1].begin = OutstandingCharacter
    #    else:                                      TriggerMap[EntryIndex-1].end   = OutstandingCharacter + 1
    #elif EntryIndex == 0:
    #    TriggerMap[EntryIndex+1].begin = OutstandingCharacter
    #elif EntryIndex == len(TriggerMap) - 1:
    #    TriggerMap[EntryIndex-1].begin = OutstandingCharacter

