/* Copyright (c) 2014 SRI International and GPC
 * Developed under DARPA contract N66001-11-C-4022.
 * Authors:
 *   Joshua Joy (JJ, jjoy)
 *   Mark-Oliver Stehr (MOS)
 */

#ifndef NETWORKCODINGCONSTANTS_H_
#define NETWORKCODINGCONSTANTS_H_

const int NETWORKCODING_FIELDSIZE=8;
//const int NETWORKCODING_BLOCKSIZE=2048;

const char* const HAGGLE_ATTR_NETWORKCODING_NAME = "_NC_BLOCK_";

const char* const HAGGLE_ATTR_PARENT_DATAOBJECT_ID = "_NC_ORIG_ID_";
const char* const HAGGLE_ATTR_NETWORKCODING_PARENT_ORIG_LEN = "_NC_ORIG_DATA_LEN_";
const char* const HAGGLE_ATTR_NETWORKCODING_PARENT_ORIG_NAME = "_NC_ORIG_FILE_NAME_";
const char* const HAGGLE_ATTR_NETWORKCODING_PARENT_CREATION_TIME = "_NC_ORIG_CREATE_TIME_";
const char* const HAGGLE_ATTR_NETWORKCODING_PARENT_ORIG_SIGNEE = "_NC_ORIG_SIGNEE_";
const char* const HAGGLE_ATTR_NETWORKCODING_PARENT_ORIG_SIGNATURE = "_NC_ORIG_SIGNATURE_";

#endif /* NETWORKCODINGCONSTANTS_H_ */
