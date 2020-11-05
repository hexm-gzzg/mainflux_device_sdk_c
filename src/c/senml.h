#ifndef SENML_H
#define SENML_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


#define SENML_SUPPORTED_VERSION (5)    //!< The highest SenML version this implementation supports


#define SJ_VERSION       "bver" //!< Key for the version attribute in JSON documents
#define SJ_BASE_NAME     "bn"   //!< Key for the base name attribute in JSON documents
#define SJ_BASE_TIME     "bt"   //!< Key for the base time attribute in JSON documents
#define SJ_BASE_UNIT     "bu"   //!< Key for the base unit attribute in JSON documents
#define SJ_BASE_VALUE    "bv"   //!< Key for the base value attribute in JSON documents
#define SJ_NAME          "n"    //!< Key for the name attribute in JSON documents
#define SJ_UNIT          "u"    //!< Key for the unit attribute in JSON documents
#define SJ_VALUE         "v"    //!< Key for the value attribute in JSON documents
#define SJ_STRING_VALUE  "vs"   //!< Key for the string value attribute in JSON documents
#define SJ_BOOL_VALUE    "vb"   //!< Key for the boolean value attribute in JSON documents
#define SJ_VALUE_SUM     "s"    //!< Key for the value sum attribute in JSON documents
#define SJ_TIME          "t"    //!< Key for the time attribute in JSON documents
#define SJ_UPDATE_TIME   "ut"   //!< Key for the update time attribute in JSON documents
#define SJ_DATA_VALUE    "vd"   //!< Key for the data value attribute in JSON documents


#define SC_VERSION       (-1)   //!< Key for the version attribute in CBOR documents
#define SC_BASE_NAME     (-2)   //!< Key for the base name attribute in CBOR documents
#define SC_BASE_TIME     (-3)   //!< Key for the base time attribute in CBOR documents
#define SC_BASE_UNIT     (-4)   //!< Key for the base unit attribute in CBOR documents
#define SC_BASE_VALUE    (-5)   //!< Key for the base value attribute in CBOR documents
#define SC_NAME          (0)    //!< Key for the name attribute in CBOR documents
#define SC_UNIT          (1)    //!< Key for the unit attribute in CBOR documents
#define SC_VALUE         (2)    //!< Key for the value attribute in CBOR documents
#define SC_STRING_VALUE  (3)    //!< Key for the string value attribute in CBOR documents
#define SC_BOOL_VALUE    (4)    //!< Key for the boolean value attribute in CBOR documents
#define SC_VALUE_SUM     (5)    //!< Key for the value sum attribute in CBOR documents
#define SC_TIME          (6)    //!< Key for the time attribute in CBOR documents
#define SC_UPDATE_TIME   (7)    //!< Key for the update time attribute in CBOR documents
#define SC_DATA_VALUE    (8)    //!< Key for the data value attribute in CBOR documents


/*! The different data types SenML supports */
typedef enum {
	SENML_TYPE_UNDEF  = 0,  //!< Placeholder for when no value is provided
	SENML_TYPE_FLOAT  = 1,  //!< Indicates a float value
	SENML_TYPE_STRING = 2,  //!< Indicates a string value
	SENML_TYPE_BOOL   = 3,  //!< Indicates a boolean value
	SENML_TYPE_BINARY = 4   //!< Indicates a binary (data) value
} senml_value_type_t;


/*! Buffer that holds arbitrary binary data */
typedef struct {
	uint8_t *p;     //!< Pointer to the actual data
	size_t   len;   //!< Size of the data in bytes
} senml_bin_data_t;


/*! struct that contains base information which applies to all subsequent entries */
typedef struct {
	uint8_t             version;         //!< SenML version of this pack
	char               *base_name;       //!< Concatenated with name attribute gives sensor's UID
	double              base_time;       //!< Update time values are relative to this time
	char               *base_unit;       //!< When all values have the same unit, set this field
	senml_value_type_t  base_value_type; //!< Indicates which type the base value is of
	union {
		double            base_value_f;    //!< A base value of type float
		char             *base_value_s;    //!< A base value of type string
		bool              base_value_b;    //!< A boolean base value (FIXME is this sensible?)
		senml_bin_data_t  base_value_bin;  //!< A binary base value (FIXME is this sensible?)
	} base_value;
} senml_base_info_t;


/*! struct that contains the values of a SenML record */
typedef struct {
    senml_base_info_t  *base_info;  //!< Pointer to the base info, may be NULL

	char              *name;        //!< Sensor's name, will be appended to base name if provided
	char              *unit;        //!< Unit of the measurement
	double             time;        //!< Time (seconds since unix epoch) of the measurement
	unsigned int       update_time; //!< Time before sensor provides an updated measurement
	// [type]          value_sum    // FIXME clear definition missing
	senml_value_type_t value_type;  //!< Indicates which type the base value is of
	union {
		double            value_f;   //!< A value of type float
		char             *value_s;   //!< A value of type string
		bool              value_b;   //!< A boolean value
		senml_bin_data_t  value_bin; //!< A binary value (FIXME don't know how to handle this yet)
	} value;
} senml_record_t;


/*! struct that holds a SenML pack (optional base info and 1..n records) */
typedef struct {	
	senml_record_t     *records;    //!< Pointer to the beginning of an array of records
	size_t              num;        //!< Number of records in the array
} senml_pack_t;



////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Decodes a SenML pack in JSON format and writes the results in \p pack. The memory necessary to 
 * store the decoded records must be allocated in advance.
 * @param[in]     input The JSON document containing the SenML pack.
 * @param[in,out] pack The <code>senml_pack_t</code> structure in which the records will be stored.
 * <code>pack->num</code> must indicate the number of <code>senml_record_t</code> elements
 * <code>pack->records</code> can hold.
 * @return 0 on success, -1 if \p input is not a valid SenML document, or -2 if \p input contains
 * more records than <code>pack->records</code> can hold.
 */
// int senml_decode_json_s(const char *input, senml_pack_t *pack);


/**
 * Creates a SenML document in JSON format. The memory necessary to store the resulting JSON
 * document must be allocated in advance.
 * @param[in] pack The <code>senml_pack_t</code> elements that contains the SenML records.
 * @param[in,out] output The char buffer that will contain the finished JSON document.
 * @param[in] len The size of the allocated memory that \p output points to in bytes.
 * @return 0 on success, -1 if \p pack contains invalid data or options, or -2 if not enough memory
 * was allocated.
 */
// int senml_encode_json_s(const senml_pack_t *pack, char *output, size_t len);
////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * Decodes a SenML pack in JSON format and writes the results in \p pack. The memory necessary to 
 * store the decoded records will be allocated automatically.
 * @param[in] input The JSON document containing the SenML pack.
 * @return A valid pointer to a <code>senml_pack_t</code> elements, or NULL on failure.
 */
senml_pack_t *senml_decode_json(char *input, size_t len);

senml_pack_t *senml_normalize(senml_pack_t *pack);
senml_pack_t *senml_create_pack(int record_num);
void senml_destroy_pack(senml_pack_t *pack);

/**
 * Creates a SenML document in JSON format. The memory necessary to store the resulting JSON 
 * document will be allocated automatically.
 * @param[in] pack The <code>senml_pack_t</code> elements that contains the SenML records.
 * @return A valid pointer to the finished JSON document, or NULL on failure.
 */
char *senml_encode_json(const senml_pack_t *pack);

/**
 * Pretty prints the base info.
 * @param[in] base_info
 */
void senml_print_base_info(const senml_base_info_t *base_info);


/**
 * Pretty prints a record.
 * @param[in] record
 */
void senml_print_record(const senml_record_t *record);


/**
 * Pretty prints a pack (base info if available and all records).
 * @param[in] pack
 */
void senml_print_pack(const senml_pack_t *pack);


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // SENML_H
