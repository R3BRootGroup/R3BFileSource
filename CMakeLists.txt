##############################################################################
#   Copyright (C) 2023 GSI Helmholtzzentrum für Schwerionenforschung GmbH    #
#   Copyright (C) 2023-2025 Members of R3B Collaboration                     #
#                                                                            #
#             This software is distributed under the terms of the            #
#                 GNU General Public Licence (GPL) version 3,                #
#                    copied verbatim in the file "LICENSE".                  #
#                                                                            #
# In applying this license GSI does not waive the privileges and immunities  #
# granted to it by virtue of its status as an Intergovernmental Organization #
# or submit itself to any jurisdiction.                                      #
##############################################################################

if(WITH_UCESB)
add_subdirectory(unpack_test/s515)
add_subdirectory(unpack_test/g249)
endif(WITH_UCESB)
