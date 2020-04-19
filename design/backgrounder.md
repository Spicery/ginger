# Background Concepts of Ginger

This note sets out the background concepts of Ginger that are not commonplace and need separate explanation.

## Dynamic, Updateable, Locked, Sealed, Petrified

Ginger is a fully object-oriented programming language, so that all values can legitimately be called Objects and the terms `value` and `object` are interchangeable. However, Ginger makes you, the programmer, be explicit about how mutable you want your Objects to be - and nudges you towards using immutable Objects by default. This is because immutable Objects enjoy cheaper implementations and lower overheads than mutable Objects. And it also encourages a simpler, performant and more robust programming style.

Ginger distinguishes between five levels of mutability:

* Dynamic - a mutable state where you can change anything, including adding, deleting and updating members.
* Updateable - a more limited mutable state, you cannot resize, add or take away existing members but you can swap them for something else.
* Locked aka Frozen - a reversible state of immutability, where you cannot make any changes to this object. The members of the object may still individually be mutable, although you cannot change their membership. Locking an object may optionally optimise the implementation.
* Sealed - an irreversible immutable state, you cannot make any changes to this object but you can to members. Ginger treats this as the default e.g. `Vector` allocates a sealed object. Mutable classes may support sealing of their instances, which may optionally optimise the implementation.
* Petrified - a recursively sealed state, you cannot make any changes to this object _nor_ any member even indirectly because all members are petrified too.

## Membership: Logical Objects and Concrete Instances

To make sense of these different levels of mutability, we need to put the concept of 'members' on a firmer foundation. To do this we need to appreciate that the word "object" is used for two different ideas in pure object-oriented computing.

1. An instance of a class, typically implemented as a contiguous block of memory in the 'heap'.
2. An implementation of an abstract type, that might be realised as single instance or a tightly related group of instances.

It is the second definition that is relevant for Ginger; when we say "object" we are using the concept of a value that might be implemented as a set of instances. For the most part we don't care how an object is implemented. In contrast, when we say "instance" we are bypassing the abstract type and looking at how they are stored in memory.

For example, a dynamic vector (`DynamicVector`) might be implemented as a record that points to an underlying updateable-vector and contains the start and end points of the members. Both the record and the vector are separate concrete instances that belong to the same logical object. 

## Handles, Parts and Members

The whole group of instances of a single object is usually represented by one of the instances (the 'handle') and the rest are concealed. These instances are said to be "parts" of the object and the boundary of the object includes those instances. Continuing with our example of the `DynamicVector`, the object is fronted by the record and the underlying vector is concealed. 

The members of an object are, of course, other objects that are external and not part of the same object (unless it points to itself!) However, the relationship between the object and one of its members *is* included. Consider, for example, a `DynamicVector` V that contains a string S; although S is a completely separate object the fact it is contained by V is V's business and no one else's. In a nutshell, you cannot add, remove or alter the members of V without changing V.

## Handles Are Shared, Mutable Parts Aren't, Petrified Parts Can Be

The implementation of an object as a group of cooperating instances usually involves deciding on which class will be instantiated as the handling-instance and which classes will be instantiated as the parts. These decisions define the logical boundary of the object. Ginger allows you to annotate fields as 'parts-of' slots but it should be emphasised that this is for readability. The boundary is fundamentally a design principle that is layered on top of the concrete instances. 

There are some clear guidelines for the design of objects that you should follow, with rare exceptions. Handles to objects are ideal for sharing & members are always represented by a handle-instance. Parts that get altered should never even be exposed - nor should locked/sealed parts be. The exception is for petrified parts, which may also be shared.

When objects are copied, the handle-and-parts are all cloned, although petrified parts can be skipped. This means that the new object starts with the same members but all changes to the membership are local to the new object and do not affect the old object (or any other) at all. 

## Objects as Parts

GOT HERE

