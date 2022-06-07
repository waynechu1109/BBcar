# final_project

(1) how to setup and run the program

i. 裝置安裝：
    BB Car 上需要安裝 qti sensor, Xbee Chip, Ping, Optical Encoder, Continuous Servo
    並按照他們的接線方式妥善接上 Mbed Board。

ii. 放置BB Car
    將 BB Car 對齊黑線放置

iii. 開啟行動電源
    開啟行動電源後，Mbed Board 就會啟動，
    BB Car 也會隨即開始向前走，並依指示走到終點。


(2) what are the results

i. 將車子放上軌道，並開啟行動電源，車子就會自行開始沿著地上的軌跡行走。

ii. 當車子與到十字路口時，車子就會依據在該路口前遇到的轉彎指令來判斷這個路口該左轉還是右轉。

iii. 如果車子駛入前方有障礙物的岔路時，車子會回轉，沿著原路回到上一個路口，並繼續往終點前進。

iv. 將數據線接上行進中的車子，並在電腦上執行car_control.py，
    我們就能夠從terminal進入能夠與車子對話的介面。
    如果此時我們在鍵盤上按下"d"，
    我們就能夠在Mbed studio的視窗中看見車子目前累積的行走距離以及目前的行駛速度。

    如果按下"q"，就能退出這個介面。
