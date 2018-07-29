#include "ofApp.h"

//using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup(){
    //画面の基本設定
    ofBackground(0, 0, 0);
    ofEnableAlphaBlending();
    ofSetFrameRate(60);
    
    //カメラから映像を取り込んで表示
    cam.setVerbose(true);
    cam.initGrabber(320,240);
    
    //使用する画像の領域を確保
    srcImg.allocate(320,240);
    shadowImg.allocate(320,240);
    
    //変数の初期化
    background_color = 255;
    thresh = 140;
    videoMode = 1;
    width = srcImg.width;
    height = srcImg.height;
    dx = 0;
    dy = 0;
    rotate = 0;
    time_flip = 0;
    turn_count = 0;
    time_turn = 0;
    
    jump = false;
    jumping = false;
    falling = false;
    up = false;
    flip = false;
    back = false;
    turnLeft1 = false;
    turnLeft2 = false;
    flip_timer = false;
    turn_timer = false;
}

//--------------------------------------------------------------
void ofApp::update(){
    bool bNewFrame; //画像が更新されたかどうか
    
    //新規フレームの取り込みをリセット
    bNewFrame = false;
    
    //カメラから新規フレーム取り込み
    cam.update();
    //新規にフレームが切り替わったか判定
    bNewFrame = cam.isFrameNew();
    
    //フレームが切り替わった際のみ画像を解析
    if (bNewFrame){
        IplImage shadowIpl;
        cv::Mat shadowCv;
        
        int idx, idx2;
        int shadow[width][height];
        int center_x = 0, center_y = 0;
        int sum = 1;
        int move;
        int x2 = 0, y2 = 0;
        int lw, rw, uh, dh;
        int low_x = width - 1, low_y = height - 1;
        int high_x = 0, high_y = 0;
        
        // リサイズデータ
        lw = width / 16;
        rw = width * 15 / 16;
        uh = height / 16;
        dh = height;
        
        //取り込んだフレームを画像としてキャプチャ
        srcImg.setFromPixels(cam.getPixels().getData(), 320, 240);
        
        //カラーのイメージをグレースケールに変換
        shadowImg = srcImg;
        
        // cvImgに変換
        shadowIpl = toCv(shadowImg);
        shadowCv = toCv(shadowImg);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                if ((int)(unsigned char)shadowIpl.imageData[idx] <= thresh) {
                    shadowIpl.imageData[idx] = 150;
                    shadow[x][y] = 1;
                    
                    center_x += x;
                    center_y += y;
                    sum++;
                    
                    if (low_x > x) {
                        low_x = x;
                        low_y = y;
                    }
                    if (high_x < x) {
                        high_x = x;
                        high_y = y;
                    }
                } else {
                    shadowIpl.imageData[idx] = background_color;
                    shadow[x][y] = 0;
                }
            }
        }
        center_x /= sum;
        center_y /= sum;
        
        // 判定 =====================================================================
        bool judge_jump = true;
        bool judge_fall = true;
        bool judge_flip = true;
        
        int count = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (shadow[x][y] == 1) {
                    count++;
                }
            }
        }
        
        // 画面外リセット #############################################################
        if (count < 10) {
            jump = false;
            jumping = false;
            falling = false;
            up = false;
            flip = false;
            back = false;
            turnLeft1 = false;
            turnLeft2 = false;
            flip_timer = false;
            turn_timer = false;
            
            cv::Point2f original[] = {cv::Point2f(0, 0), cv::Point2f(width, 0), cv::Point2f(0, height), cv::Point2f(width, height)};
            cv::Point2f translate[] = {cv::Point2f(lw, uh), cv::Point2f(rw, uh), cv::Point2f(lw, dh), cv::Point2f(rw, dh)};
            
            cv::Mat homo_matrix = cv::getPerspectiveTransform(original, translate);
            warpPerspective(shadowCv, shadowCv, homo_matrix, shadowCv.size());
            
            return;
        }
        // ############################################################# 画面外リセット
        
        if (count < 1000) {
            judge_jump = false;
            judge_fall = false;
            judge_flip = false;
        }
        
        // JUMP
        for (int x = 0; x < width; x++) {
            if(shadow[x][height * 15 / 16] == 1) {
                judge_jump = false;
                break;
            }
        }
        if (judge_jump) {
            if (!up) {
                jump = true;
                jumping = true;
            }
        }
        
        // FALL
        for (int x = 0; x < width; x++) {
            if(shadow[x][height * 9 / 16] == 1) {
                judge_fall = false;
                break;
            }
        }
        if (judge_fall) {
            if (up) {
                falling = true;
            }
        }
        
        // FLIP
        int count_change = 0;
        int blank = 0;
        for (int x = 1; x < width; x++) {
            if (count_change == 2) {
                blank++;
                if (shadow[x][height * 1 / 10] != shadow[x - 1][height * 1 / 10] && blank > width / 10) {
                    count_change++;
                }
            } else {
                if(shadow[x][height * 1 / 10] != shadow[x - 1][height * 1 / 10]) {
                    count_change++;
                }
            }
        }
        if (count_change == 4 && !flip_timer) {
            if (!jumping && !falling) {
                flip = !flip;
                flip_timer = true;
            }
        }
        if (flip_timer) {
            time_flip++;
            if (time_flip > 50) {
                flip_timer = false;
                time_flip = 0;
            }
        }
        
        // TURN
        if ((high_x - low_x) > width / 2 && abs(high_y - low_y) < 20 && high_y < height * 3 / 4 && !turn_timer) {
            turn_count++;
            if (turn_count > 20) {
                turnLeft1 = true;
                turn_timer = true;
            }
        } else {
            turn_count = 0;
        }
        
        if (turn_timer) {
            time_turn++;
            if (time_turn > 100) {
                turn_timer = false;
                time_turn = 0;
            }
        }
        
        // ===================================================================== 判定
        
        
        // SYMMETRY, FLIP =====================================================================
        move = (center_x - (width / 2)) * 2;
        if (flip) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    idx = y * width + x;
                    shadowIpl.imageData[idx] = background_color;
                }
                for (int x = width - 1; x >= 0; x--) {
                    idx = y * width + x;
                    idx2 = y2 * width + x2;
                    
                    if (shadow[x2][y2]) {
                        // flip
                        if (flip && move + x >= 0 && move + x < width) {
                            shadowIpl.imageData[idx + move] = 150;
                        }
                    }
                    x2++;
                }
                
                // shadow[][]の更新
                for (int x = 0; x < width; x++) {
                    idx = y * width + x;
                    if ((int)(unsigned char)shadowIpl.imageData[idx] == 150) {
                        shadow[x][y] = 1;
                    } else {
                        shadow[x][y] = 0;
                    }
                }
                
                x2 = 0;
                y2++;
            }
        }
        // ===================================================================== SYMMETRY, FLIP
        
        // JUMP =====================================================================
        if (jump) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    idx = y * width + x;
                    shadowIpl.imageData[idx] = 255;
                }
            }
            
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    idx = y * width + x;
                    if (x + dx < width && y + dy < height) {
                        if (shadow[x][y + dy] == 1) {
                            shadowIpl.imageData[idx] = 150;
                        }
                    }
                }
            }
            
            if (jumping) {
                dy += 20;
                if (dy >= height) {
                    jumping = false;
                    up = true;
                }
            }
            
            if (falling) {
                dy -= 20;
                if (dy <= 0) {
                    falling = false;
                    jump = false;
                    up = false;
                }
            }
        }
        // ===================================================================== JUMP
        
        // 裏返す
        if (back) {
            cv::flip(shadowCv, shadowCv, 1);
        }
        
        // 回転3D
        if (turnLeft1 || turnLeft2) {
            if (rotate < width * 7 / 16) {
                rotate += 5;
                
                if (turnLeft2 && rotate >= 0) {
                    rotate = 0;
                    turnLeft2 = false;
                }
                
            } else {
                rotate =  -1 * width * 7 / 16;
                turnLeft1 = false;
                turnLeft2 = true;
                back = !back;
            }
        }
        
        cv::Point2f original[] = {cv::Point2f(0, 0), cv::Point2f(width, 0), cv::Point2f(0, height), cv::Point2f(width, height)};
        cv::Point2f translate[] = {cv::Point2f(lw + abs(rotate), uh + rotate * 0.4 / 2), cv::Point2f(rw - abs(rotate), uh - rotate * 0.4 / 2), cv::Point2f(lw + abs(rotate), dh - rotate * 0.4 / 2), cv::Point2f(rw - abs(rotate), dh + rotate * 0.4 / 2)};
        
        cv::Mat homo_matrix = cv::getPerspectiveTransform(original, translate);
        warpPerspective(shadowCv, shadowCv, homo_matrix, shadowCv.size());
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255, 255, 255);
    
    //現在のモードに応じて、表示する映像を切り替え
    switch (videoMode) {
            
        case 1:
            //影映像
            shadowImg.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
            
        default:
            //カラー映像
            srcImg.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    //キー入力でモード切り替え
    switch (key){
        case '0':
            videoMode = 0;
            break;
            
        case '1':
            videoMode = 1;
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    switch (key) {
        case OF_KEY_UP:
            if (!up) {
                jump = true;
                jumping = true;
            }
            break;
        case OF_KEY_DOWN:
            if (up) {
                falling = true;
            }
            break;
        case 'z':
            turnLeft1 = true;
            break;
            
        case 'x':
            if (!jumping && !falling) {
                flip = !flip;
            }
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

