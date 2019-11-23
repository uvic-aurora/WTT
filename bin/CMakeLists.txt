set(programs
    wtt_noise_removal_demo
    wtt_demo
    wtt_gen_code_cov
    wtt_run_tests
    run_demo)
  
install(PROGRAMS ${programs} DESTINATION ${CMAKE_INSTALL_BINDIR})