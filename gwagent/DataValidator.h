/**
 * @file DataValidator.h
 * @brief for checking if deivces data collected is valid
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-04-28
 */
#ifndef _DATA_VALIDATOR_H
#define _DATA_VALIDATOR_H

#include "easylogging++.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"  
#include "rapidjson/schema.h"  
#include "rapidjson/error/en.h"
#include <iostream>               
#include <string>
#include <memory>

class DataValidator {

    public:
        DataValidator():bInit(false), schema(nullptr) {
        }

        ~DataValidator() {
        }

        void init() {
            const char * schemaJson = "{\"type\": \"array\",  \"items\":{\"properties\": {\"devid\":{\"type\":\"string\"}, \"devname\":{\"type\":\"string\"}, \"prop\":{\"type\":\"string\"}},\"type\": \"object\",\"required\": [\"devid\", \"devname\", \"prop\"]}}";

            rapidjson::Document sd;    
            if (false == sd.Parse(schemaJson)) {
                // the schema is not a valid JSON.
                LOG(ERROR)<<"Parse schemaJson fail";
                bInit = false;
                return;
            }

            schema = std::make_shared<rapidjson::SchemaDocument>(sd); // Compile a Document to SchemaDocument
            bInit = true;
        }


        void fin() {

        }

        bool validJsonData(const std::string & data) {
            if (false == bInit) {
                return false;
            }

            rapidjson::Document d;
            if (false == d.Parse(data.c_str())) {
                // the input is not a valid JSON.
                LOG(ERROR)<<"Parse inputJson fail";
                return false;
            }

            rapidjson::SchemaValidator validator(*schema);
            if (false == d.Accept(validator)) {
                // Input JSON is invalid according to the schema
                // Output diagnostic information
                rapidjson::StringBuffer sb;
                validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
                LOG(ERROR)<<"Invalid schema: "<<sb.GetString();
                LOG(ERROR)<<"Invalid keyword: "<<validator.GetInvalidSchemaKeyword();
                sb.Clear();
                validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
                LOG(ERROR)<<"Invalid document: "<<sb.GetString();
                return false;
            }         
            else {    
                LOG(DEBUG)<<"meet schema";
            } 

            return true;  
        }
    private:
        bool bInit;
        std::shared_ptr<rapidjson::SchemaDocument> schema;
    private:
        DataValidator(const DataValidator&) =delete;
        DataValidator& operator=(const DataValidator&) =delete;
        DataValidator(DataValidator &&) =delete;
        DataValidator& operator=(DataValidator &&) =delete;
};

#endif

