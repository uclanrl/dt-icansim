dt-icansim
==========

DT-ICANSIM: A Delay-Tolerant Information-Centric Ad-Hoc Networking Simulation Module


UCLA Network Research Lab (NRL) simulation of DT-ICAN. http://nrlweb.cs.ucla.edu/
Techincal report can be found here: http://nrlweb.cs.ucla.edu/publication/show/787

This software works with QualNet and has been tested with QualNet 6.1. Users must have a valid license of QualNet to use it. 

Usage: the following assumes you already have a QualNet distribution.
1. Copy user_models/ to Qualnet/6.1/user_models
2. Modify Qualnet Makefile to include user_models, that is, add the following two lines to Makefile-addons-unix:
  # USER_MODELS library
  include ../libraries/user_models/Makefile-unix

  or add these to Makefile-addons-windows:
  # USER_MODELS library
  include ../libraries/user_models/Makefile-windows  
3. Add messages listed in include/readme.txt in include/api.h
4. Compile
5. Sample configs for DT-ICANSIM are included in dtnconfig/ 
    qualnet sample-dtn.config
        
This software is free for research purpose. Academic publications that use DT-ICANSIM are requested to cite the following publication:

@TECHREPORT{yu13dtican,
     AUTHOR = "Yu-Ting Yu, Joshua Joy, Mario Gerla, and M. Y. Sanadidi",
      TITLE = "DT-ICAN: A Delay-tolerant Information-Centric Ad-Hoc Network",
INSTITUTION = "Department of Computer Science, University of California, Los Angeles",
     NUMBER = "TR130017",
       YEAR = "2013"
} 
 
 