#
#   Copyright 2013 Per Sigmond <per@sigmond.no>
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

%option parameter_set personnel short_name pers;

%h_begin
#define EMPLOYEE_NUMBER_UNDEFINED 0
#define EMPLOYEE_NUMBER_BASE 100
#define EMPLOYEE_NUMBER_MAX 1000
%h_end

parameter_set personnel {

    prefix pers;

    parameters {
        /** A name */
        string NameString max 50;

        /** Calendar year */
        int Year min 1900;

        /** Month number in year (january = 1) */
        int Month range(1..12);

        /** Day of month */
        int Day range(1..31);

        /** Unique number that identifies an employee */
        uint16 EmployeeNumber;

        /** Data type to hold binary data for a picture */
        uint8_array PictureData;

        /** Encoding format for a picture */
        enum PictureFormat {
            gif, /**< GIF format */
            jpg  /**< JPEG format */
        };

        /** Data type to hold a picture */
        bag Picture {
            PictureFormat format, /**< Format of the binary data */
            PictureData data      /**< The binary picture data */
        };

        /** Data type to hold the full name of a person */
        bag Name {
            NameString first,   /**< First name */
            NameString *middle, /**< Middle name */
            NameString last     /**< Last name */
        };

        /** Data type to hold a date */
        bag Date {
            Year year,   /**< Calendar year */
            Month month, /**< Month of the year */
            Day day      /**< Day of month */
        };

        /** The gender of a person */
        enum Gender {
            female, /**< The person is a woman or a girl */
            male    /**< The person is a man or a boy */
        };

        /** Holds data for a person */
        bag Person {
            Name name,    /**< Full name */
            Date born,    /**< Date of birth */
            Date *died,   /**< Date of death (if applicable) */
            Gender gender /**< Gender of the person */
        };

        /** Holds data for an employee (inherits Person) */ 
        bag Employee : Person {
            EmployeeNumber number, /**< Unique employee identity */
            Date hired,            /**< Date the employee was hired */
            Date *left,            /**< Date the employee left (if applicable) */
            Picture *picture       /**< A photo of the employee */
        };

        /** Holds client data */
        addr Userdata;

        /** Error numbers */
        signed_enum8 Error {
            success = 0,    /**< No error */
            general = -128, /**< General error */
            parameter,      /**< Error in the request parameters */
            not_found,      /**< Entry not found */
            memory          /**< Memory error */
        };
        /** Holds a string with extended error info (if applicable) */
        string ErrorInfo;

        /** Reason for server shutdown */
        enum8 ShutdownReason {
            unknown = 1, /**< Reason is unknown */
            maintenance, /**< Server down for maintenance */
            emergency    /**< Emergency shutdown at critical errror */
        };
    };
};

category persfile using personnel {

    /** Generic request format */
    command_bag Req {
        Userdata *userdata /**< Pointer to client-private data */
    };

    /** Generic response format */
    response_bag Resp {
        Error error,          /**< Error number */
        ErrorInfo *errorinfo, /**< Extended info about error */
        Userdata *userdata    /**< Pointer to client-private data */
    };

    /** Generic event format */
    event_bag Evt {
    };

    commands {

        /** Add an employee record
        * @param employee The employee record to add
        * @param number Unique number given to employee 
        */
        Add(Employee employee, out EmployeeNumber number);

        /** Get employee record from number
        * @param number Unique employee number
        * @param employee The retrieved employee record
        */
        Get(EmployeeNumber number, out Employee *employee);

        /** Delete an employee record
        * @param number unique employee number
        */
        Delete(EmployeeNumber number);

        /** Find employees from name
        * @param first First name
        * @param middle Middle name
        * @param last Last name
        * @param employees List of employees matching the name
        */
        Find(NameString *first,
             NameString *middle,
             NameString *last,
             out Employee *employees[]
            );
    };

    events {
        /** The server has shut down
        * @param reason The reason why the server was shut down
        */
        ServerShutdown(out ShutdownReason reason);
    };
};
