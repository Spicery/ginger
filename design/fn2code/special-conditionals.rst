%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Specialised Conditional Instructions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

The following instructions are designed for common conditions in if-the-else and
while-loops. Although they are not well-named, the files are named 
systematically with the format

	{comparison_op}_{ss|si}.irrr

The instructions have three fields following the instruction.

	Local slot
	Local slot _or_ numeric constant
	Jump distance

They fall into two groups. This first group compares a local-variable slot
against a constant. The comparison operators should be self-explanatory, using
the usual abbreviations e.g. gte = greater-than-or-equal-to.

[Note to SFKL: irrr should be ircr. When I switched from exclusively Small
to numeric values I needed to fix this. Doubles, in particular, won't work.]

 *  eq_si.irrr
 *  gt_si.irrr
 *  gte_si.irrr
 *  lt_si.irrr
 *  lte_si.irrr
 *  neq_si.irrr

 The second group compare two local-variable slots with each other.

 *  eq_ss.irrr
 *  gt_ss.irrr
 *  gte_ss.irrr
 *  lt_ss.irrr
 *  lte_ss.irrr
 *  neq_ss.irrr

