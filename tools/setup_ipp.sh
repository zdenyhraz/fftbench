#!/bin/bash

cd libs
mkdir intel
cd intel
wget https://registrationcenter-download.intel.com/akdlm/irc_nas/18748/l_ipp_oneapi_p_2021.6.0.626.sh
chmod 755 l_ipp_oneapi_p_2021.6.0.626.sh
./l_ipp_oneapi_p_2021.6.0.626.sh