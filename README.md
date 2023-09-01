# 재실 관리를 이용한 침수 감지 시스템
이 시스템은 재실관리에 침수감지 센서를 넣어 방 안의 침수를 감지하는 시스템입니다


아두이노(Admin) - test.ino

아두이노(Sensor) - iot_client_bluetooth2.ino

라즈베리파이(Server) - iot_server1.c

라즈베리파이(User or 119) - iot_client1.c


이렇게 구성된 시스템입니다

아두이노(.ino)는 그대로 넣으시면 되고, 
라즈베리파이(.c)는 컴파일 후 실행해야합니다.

//FLOODSENCE QA1.xlsx은 QA용 점검표임 

//QA점검표는 작성후 월/일 FLOODSENCE QA.xlsx파일을 만들어 올릴것  


// 파일 수정전에 반드시 원본파일을 다운로드한 이후 수정  
// 파일 수정 후에 '반드시' 컴파일 및 테스트해보기  

// 해야할 것 0831 JSHITRED  

//  -가격절감  
//     재실관리가 필수적인지 확인(출입인원만 확인하면 더욱 저가에 가능할 것으로 예정)  
//      아두이노 미니로는 안되는지 확인  

//   -정확도 상승  
//     감지 센서의 민감도를 조절할 필요가 있음  
//     정밀한 측정을 위해 초음파 센서도 같이 측정할 것을 제안  
   
//   -긴급호출 수정방향  
//     주소를 이름으로 사용하여 긴급신호를 보내는 원리이면 설치를 거부하는 곳이 있을 수 있기 때문에 MGRS와 유사하면서 일반인들은 알기 힘든 주소로 구분할 필요가 있음  
//     119에서 하는 안심콜 서비스와 연동이 가능했으면 좋겠음  
//     시간이 된다면 거주자들의 휴대폰으로 침수시 알림이 가는 기능도 만들면 좋갰음  
/*     데이터를 보내는 장소구분  
          일반상황 -> 관리자, 경비실, admin, 일반인  
          긴급상황 -> 119, 관리자  
*/  
