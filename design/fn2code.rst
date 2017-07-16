%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fn2code: Compiling fn elements to code elements
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

================================================================================
The Vision
================================================================================

Replace the <fn> ... </fn> elements in GingerXML code with <code> ... </code>
elements, whose children are <instruction/> elements.

For example::

    define f( x, y ) =>> x + y enddefine

Generates::

    <bind arity.eval="0" span="1">
        <var arity.pattern="1" def.pkg="ginger.interactive" name="f" protected="true" scope="global" uid="0"/>
        <fn arity.eval="1" name="f">
            <seq arity.pattern="2">
                <var arity.pattern="1" name="x" protected="true" scope="local" span="1" uid="1"/>
                <var arity.pattern="1" name="y" protected="true" scope="local" span="1" uid="2"/>
            </seq>
            <sysapp args.arity="2" arity.eval="1" name="+">
                <id arity.eval="1" name="x" protected="true" scope="local" span="1" uid="1"/>
                <id arity.eval="1" name="y" protected="true" scope="local" span="1" uid="2"/>
            </sysapp>
        </fn>
    </bind>

This should be turned into::

    <bind arity.eval="0" span="1">
        <var arity.pattern="1" def.pkg="ginger.interactive" name="f" protected="true" scope="global" uid="0"/>
        <fn.code args.count="2" arity.eval="1" locals.count="2" arg.count="2" name="f">
            <enter/>
            <push local="0"/>
            <push local="1"/>
            <add/>
            <return/>
        </code>
    </bind>

Or, with suitable optimisation, into::

    <bind arity.eval="0" span="1">
        <var arity.pattern="1" def.pkg="ginger.interactive" name="f" protected="true" scope="global" uid="0"/>
        <fn.code args.count="2" arity.eval="1" locals.count="2" arg.count="2" name="f">
            <check count="2"/>
            <add/>
            <sysreturn/>
        </code>
    </bind>
