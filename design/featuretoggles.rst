Notes Towards Feature Toggles to Support Language Evolution
===========================================================

Overview
--------
Programming languages need to evolve over time and that eviolution would be 
helped if  application-programmers were able to control the visibility of new 
language features. The idea behind feature-toggles is that they provide granular 
control over each new language feature.

The Prime Use Case
------------------
The scenario that feature toggles are supposed to assist is [1] an application 
programmer has a large code base consisting of 1000s of files [2] a new feature 
is added to Ginger that they need to apply incrementally to the code base. In
particular they need to be able to fluidly mix pre-change code with post-change 
code so that the code rolls over from one language version to the next without
breaking.

Language Updates
----------------
Language updates would be considered to be a collection of enabled (and 
disabled) feature toggles. So it is highly desirable that feature toggles can
be applied in groups and not just individually. Rolling over to a new version
can be done in stages, switching on features unit by unit.

Requires, Incompatible, Compatible
----------------------------------
Features are all about managing dependencies on the base version. So it is
useful to say whether or not a unit requires a feature, is incompatible with a
feature, or is compatible-but-independent with a feature.::

	+MyFeature the feature MyFeature must be enabled
	-MyFeature the feature MyFeature must be disabled
	±MyFeature the feature MyFeature can be enabled/disabled without harm

If a unit is marked as needing a feature to be enabled, the compiler must either
dynamically enable the feature for the scope of the unit or halt and raise an 
error.

Granularity of Feature Toggles
------------------------------
For a fluid adoption of a feature, we have to be able to transition at many
levels of granularity. For example, it might be the case that a feature is 
released that affects an obscure function that you have never encountered: you 
can safely mark your entire project as compatible. But perhaps you only own a
package within a project, in which case you must be able to mark the package.

For those two cases, add the setting below to the appropriate settings file::

	${FEATURE_NAME} = (true|false|maybe)

For other cases, we mark code regions.::

	#region +FeatureA -FeatureB ±FeatureC
		STATEMENTS
	#endregion

[The file is not an appropriate level of granularity because of the lack of
standard meta-information across operating systems.]

Conditionality on Feature Toggles
---------------------------------
We will often want to write code that works across more than one version. To
maximise code-sharing, we need conditional compilation to be able to change
very small sections of code, much like the #ifdef feature of the C/C++ 
preprocessor can do.

This cannot be a runtime construct as it may be gating syntax changes. So we
require a conditional compilation form that can be pre-processed::
 
	#region
	#case FeatureA or FeatureB
		...
	#case not( FeatureA )
		...
	#else
		...
	#endregion

Only one of these branches will be taken. 


Baselining
----------
As the language evolves, the old version 1 becomes less important and the new 
version 2 becomes the standard. We don't want to micromanage the features that 
have been rolled up into version 2, we just want to be able to say "based on 
version 2" instead. 

All file extensions should be capable of carrying version information, so that
the version baseline is implied by their file extension. e.g.::

	Common source code: *.ggr1.common
	Project: *.ggr2.proj
	Package: *.gg1.pkg

In this case, the file is a suitable level of granularity.

What About Projects that aren't File-System Hosted?
---------------------------------------------------
Ginger supports projects in SQLITE3 and potentially many other formats. The
basic obligation on those systems is that they serve GNX, which means that they
are free to deal with features how they wish, provided that the requirements
are pushed into GNX.::

	<seq feature.required="FeatureA" feature.incompatible="FeatureB">
		...
	</seq>

N.B. Inserting multiple 'required' will be easier when Ginger switches
GNX over to HydraXML.