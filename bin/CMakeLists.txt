set(programs
    wtl_noise_removal_demo
    wtl_demo
    wtl_gen_code_cov
    wtl_run_tests
    run_demo)
  
install(PROGRAMS ${programs} DESTINATION ${CMAKE_INSTALL_BINDIR})