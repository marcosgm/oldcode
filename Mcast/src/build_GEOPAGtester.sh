#!/bin/bash
gcc GEOPAG.c -c -I ../include -D IMPRIMELOTODO
gcc GEOPAG.o GEOPAG_tester.c -g -D NIVELES=21 -lm -Wall -I ../include -o GEOPAG_tester
                                                                                
                                                                                
                                                                                

