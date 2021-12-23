all:central.cpp clientA.cpp clientB.cpp serverT.cpp serverS.cpp serverP.cpp
	g++ -std=c++11 central.cpp -o serverC

	g++ -std=c++11 clientA.cpp -o clientA

	g++ -std=c++11 clientB.cpp -o clientB

	g++ -std=c++11 serverT.cpp -o serverT

	g++ -std=c++11 serverS.cpp -o serverS
	
	g++ -std=c++11 serverP.cpp -o serverP

.PHONY: serverC
central: 
	./serverC

.PHONY: clientA
clientA: 
	./clientA

.PHONY: clientB
clientB:
	./clientB

.PHONY: serverT
serverT:
	./serverT

.PHONY: serverS
serverS:
	./serverS

.PHONY: serverP
serverP:
	./serverP
