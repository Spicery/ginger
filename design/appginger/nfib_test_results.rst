%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
NFIB data
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

::

	define nfib( n ) => if n <= 1 then 1 else 1 + nfib( n - 1 ) + nfib( n - 2 ) endif enddefine;


	MacBook nfib(34)
						M#1(s)		M#2(s)		M#3(s)
	Mac OS X -g			 3.24		 3.71		 2.08
	Mac OS X -O3 		 1.71		 2.86		 0.72
	Ubuntu VMWare -g	 2.98		 2.13		 1.39
	Ubuntu VWWare -O	 1.16		 1.42		 0.49

	Mac OS X ruby		10.50
	Mac OS X python 	 4.20

	Mac OS X C			 0.11

	VMWare Poplog (+)				 0.51
	VMWare Poplog (fi_+)			 0.28
	VMWare Optimised C				 0.14

	[Pristine:~/tmp] steve% time ./nfib 34
	nfib(34) = 18454929

	real	0m0.120s
	user	0m0.110s
	sys		0m0.002s
 
