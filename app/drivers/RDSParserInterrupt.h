///
/// \file RDSParserInterrupt.h
/// \brief RDS Parser class definition.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
/// 
/// \details
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// History:
/// --------
/// * 01.09.2014 created and RDS sender name working.
/// * 01.11.2014 RDS time added.
/// * 27.03.2015 Reset RDS data by sending a 0 in blockA in the case the frequency changes.
/// 


#pragma once

//abstract class
class RDSParserInterrupt
{
public:
	virtual void RdsReceiveServiceName(char *name) = 0;
	virtual void RdsReceiveText(char *name) = 0;
	virtual void RdsReceiveTime(uint8_t hour, uint8_t minute) = 0;
};
