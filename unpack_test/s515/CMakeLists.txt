##############################################################################
#   Copyright (C) 2023 GSI Helmholtzzentrum für Schwerionenforschung GmbH    #
#   Copyright (C) 2023 Members of R3B Collaboration                          #
#                                                                            #
#             This software is distributed under the terms of the            #
#                 GNU General Public Licence (GPL) version 3,                #
#                    copied verbatim in the file "LICENSE".                  #
#                                                                            #
# In applying this license GSI does not waive the privileges and immunities  #
# granted to it by virtue of its status as an Intergovernmental Organization #
# or submit itself to any jurisdiction.                                      #
##############################################################################

generate_root_test_script(${R3BROOT_SOURCE_DIR}/R3BFileSource/unpack_test/s515/testunpack.C)
add_test(NAME TestUnpackS515 COMMAND ${R3BROOT_BINARY_DIR}/R3BFileSource/unpack_test/s515/testunpack.sh)
set_tests_properties(TestUnpackS515 PROPERTIES TIMEOUT "2000")
set_tests_properties(TestUnpackS515 PROPERTIES PASS_REGULAR_EXPRESSION
                                                  "Macro finished successfully.")
