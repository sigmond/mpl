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
        string NameString max 50;
        int Year min 1900;
        int Month range(1..12);
        int Day range(1..31);
        uint16 EmployeeNumber;
        uint8_array PictureData;

        enum PictureFormat {
            gif,
            jpg
        };

        bag Picture {
            PictureFormat format,
            PictureData data
        };

        bag Name {
            NameString first,
            NameString *middle,
            NameString last
        };

        bag Date {
            Year year,
            Month month,
            Day day
        };

        enum Gender {
            female,
            male
        };

        bag Person {
            Name name,
            Date born,
            Date *died,
            Gender gender
        };

        bag Employee : Person {
            EmployeeNumber number,
            Date hired,
            Date *left,
            Picture picture
        };

        addr Userdata;
        enum Error {
            success,
            parameter,
            not_found,
            memory,
            general
        };
        enum ShutdownReason {
            unknown,
            maintenance,
            emergency
        };
    };
};

category persfile using personnel {

    command_bag Req {
        Userdata *userdata
    };

    response_bag Resp {
        Error error,
        Userdata *userdata
    };

    event_bag Evt {
    };

    commands {
        Add(Employee employee, out EmployeeNumber number);
        Get(EmployeeNumber number, out Employee *employee);
        Delete(EmployeeNumber number);
        Find(NameString *first,
             NameString *middle,
             NameString *last,
             out Employee *employees[]
            );
    };

    events {
        ServerShutdown(out ShutdownReason reason);
    };
};
