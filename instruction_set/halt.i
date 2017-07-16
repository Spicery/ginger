/*****************************************************************************\
Definition
    HALT

Code
    <halt />
    
Summary
    Forces the virtual machine to exit with a failure.
    
Unchecked Precondition
    None
    
Exceptions (Checked Preconditions)
    None
    
Result (Postcondition)      
    End of the VM session.

\*****************************************************************************/

#ifdef DBG_SPECIAL  
    std::cout << std::endl << "halt" << std::endl;
#endif
std::cerr << "Compilation error - fell off the end of a function" << std::endl;
exit( EXIT_FAILURE );
