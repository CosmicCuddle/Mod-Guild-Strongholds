#include "StrongholdVisitTicketRead.h"
#include <iostream>
#include <limits>
#include <string>
using namespace NaxxGuildStrongholds;
int main()
{
    int n=0,fail=0;
    auto check=[&](bool ok,char const* label){
        ++n;if(!ok){++fail;std::cerr<<"FAIL "<<label<<'\n';}
    };
    VisitRecord row;
    row.CharacterGuid=10;
    row.OriginalGuild={45,1791000000};
    row.SessionKey="secret-nonce_01";
    row.PropertyKey="human";
    row.ReturnPoint={0,0,-8950.0f,510.0f,98.0f,1.0f};
    row.Version=12;
    row.Stage=VisitStage::Prepared;
    auto r=ReviewVisitTicketRow(10,true,true,row);
    check(r.Status==VisitTicketReadStatus::Prepared,"Prepared row recognised");
    check(!r.AuthorisesTeleport && !r.AuthorisesClear &&
          !r.ProvesSafeReturn,"No privileges from any row");
    row.Stage=VisitStage::Inside;
    r=ReviewVisitTicketRow(10,true,true,row);
    check(r.Status==VisitTicketReadStatus::Inside,"Inside requires exit");
    row.Stage=VisitStage::Returning;
    r=ReviewVisitTicketRow(10,true,true,row);
    check(r.Status==VisitTicketReadStatus::Returning,"Returning not completed");
    check(!r.AuthorisesClear,"Return ticket not automatically cleared");
    check(ReviewVisitTicketRow(10,false,true,row).Status==
          VisitTicketReadStatus::Disabled,"Disabled by default");
    check(ReviewVisitTicketRow(0,true,true,row).Status==
          VisitTicketReadStatus::UnknownCharacter,"Anonymous identity refused");
    check(ReviewVisitTicketRow(10,true,false,row).Status==
          VisitTicketReadStatus::NoRowOrDatabaseUnavailable,"Absent row ambiguous");
    check(ReviewVisitTicketRow(11,true,true,row).Status==
          VisitTicketReadStatus::WrongCharacter,"Foreign row denied");
    auto broken=row;
    broken.CharacterGuid=0;
    check(ReviewVisitTicketRow(10,true,true,broken).Status==
          VisitTicketReadStatus::InvalidRow,"Empty row GUID");
    broken=row;broken.OriginalGuild.CreatedAt=0;
    check(ReviewVisitTicketRow(10,true,true,broken).Status==
          VisitTicketReadStatus::InvalidRow,"Missing original guild generation");
    broken=row;broken.SessionKey="bad nonce";
    check(ReviewVisitTicketRow(10,true,true,broken).Status==
          VisitTicketReadStatus::InvalidRow,"Invalid ticket key");
    broken=row;broken.PropertyKey="../bad";
    check(ReviewVisitTicketRow(10,true,true,broken).Status==
          VisitTicketReadStatus::InvalidRow,"Invalid property key");
    broken=row;broken.ReturnPoint.InstanceId=2;
    check(ReviewVisitTicketRow(10,true,true,broken).Status==
          VisitTicketReadStatus::InvalidRow,"Unapproved instance origin");
    broken=row;broken.ReturnPoint.X=std::numeric_limits<float>::infinity();
    check(ReviewVisitTicketRow(10,true,true,broken).Status==
          VisitTicketReadStatus::InvalidRow,"Nonfinite coordinate");
    broken=row;broken.Stage=static_cast<VisitStage>(255);
    check(ReviewVisitTicketRow(10,true,true,broken).Status==
          VisitTicketReadStatus::InvalidRow,"Invalid enum stage");
    broken=row;broken.SessionKey=std::string(65,'A');
    check(ReviewVisitTicketRow(10,true,true,broken).Status==
          VisitTicketReadStatus::InvalidRow,"Overlong nonce");
    for(int i=0;i<=8;++i)
    {
        std::string const text=VisitTicketStatusText(
            static_cast<VisitTicketReadStatus>(i));
        check(text.find("secret-nonce")==std::string::npos &&
              text.find("-8950")==std::string::npos &&
              text.find("1791000000")==std::string::npos,
              "No saved location, key or guild ID in diagnostic");
    }
    if(fail)return 1;
    std::cout<<"PASS: "<<n<<" read-only visit-ticket status tests\n";
}
