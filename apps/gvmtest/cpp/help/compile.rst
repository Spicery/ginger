Command <compile> GNX </compile>
================================

The <compile> command submits raw GNX code to the Ginger virtual machine.

Raw means that it does not send the code through the usual pipeline of 
	file2gnx < SOURCE | simplifygnx -suA 
It's important to note that the GVM relies on the transformations and
decorations performed by simplifygnx. 

The GNX code is compiled into a function object in the heap. Then a pointer
to that code is inserted into the execution queue of the virtual machine.

Example
-------

The following code works without any decorations. Simplifygnx would only 
add arity markings.

	<compile>
		<sysapp name="println">
			<constant type="string" value="Hello, world!"/>
		</sysapp>
	</compile>
