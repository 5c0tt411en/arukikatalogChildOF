// ImageSaverThreadクラス（完全スレッド化版）
class ImageSaverThread : public ofThread {
public:
    ImageSaverThread() {
        // コンストラクタ
    }
    
    ~ImageSaverThread() {
        // スレッドが実行中なら停止
        if (isThreadRunning()) {
            waitForThread(true);
        }
    }
    
    // 未処理のピクセルデータと保存ファイル名をキューに追加
    void saveImageWithRotation(const ofPixels& pixels, const string& fileName, int rotationDegrees = 90) {
        // ロックを取得
        std::unique_lock<std::mutex> lock(mutex);
        
        // ピクセルデータと保存先ファイル名、回転角度のセットを作成
        SaveItem item;
        item.pixels = pixels;  // ピクセルデータをコピー
        item.fileName = fileName;
        item.rotationDegrees = rotationDegrees;
        
        // キューに追加
        saveQueue.push(item);
    }
    
    // スレッドの開始
    void start() {
        startThread();
    }
    
    // スレッドの停止
    void stop() {
        waitForThread(true);
    }
    
private:
    // 保存対象のピクセルデータとファイル名、回転角度を保持する構造体
    struct SaveItem {
        ofPixels pixels;
        string fileName;
        int rotationDegrees;
    };
    
    // ピクセルデータを90度回転させる関数（OpenGLに依存しない実装）
    ofPixels rotatePixels90(const ofPixels& source, bool clockwise) {
        int srcWidth = source.getWidth();
        int srcHeight = source.getHeight();
        int numChannels = source.getNumChannels();
        
        // 回転後のピクセル配列を作成（幅と高さが入れ替わる）
        ofPixels result;
        result.allocate(srcHeight, srcWidth, numChannels);
        
        for (int y = 0; y < srcHeight; y++) {
            for (int x = 0; x < srcWidth; x++) {
                int newX, newY;
                
                if (clockwise) {
                    // 時計回り90度
                    newX = srcHeight - 1 - y;
                    newY = x;
                } else {
                    // 反時計回り90度
                    newX = y;
                    newY = srcWidth - 1 - x;
                }
                
                // 各チャンネルのデータをコピー
                for (int c = 0; c < numChannels; c++) {
                    result.setColor(newX, newY, source.getColor(x, y));
                }
            }
        }
        
        return result;
    }
    
    // 指定した角度（90度単位）でピクセルを回転させる関数
    ofPixels rotatePixelsByDegrees(const ofPixels& source, int degrees) {
        // 角度を正規化（0, 90, 180, 270のいずれか）
        int normalizedDegrees = ((degrees % 360) + 360) % 360;
        
        ofPixels result = source;  // デフォルトは元のピクセル
        
        if (normalizedDegrees == 90) {
            result = rotatePixels90(source, true);
        } else if (normalizedDegrees == 180) {
            // 180度回転（90度を2回適用）
            ofPixels temp = rotatePixels90(source, true);
            result = rotatePixels90(temp, true);
        } else if (normalizedDegrees == 270) {
            // 270度回転（90度を反時計回り = 270度時計回り）
            result = rotatePixels90(source, false);
        }
        
        return result;
    }
    
    // スレッドの実行関数
    void threadedFunction() {
        while (isThreadRunning()) {
            // ロックを取得
            std::unique_lock<std::mutex> lock(mutex);
            
            // キューが空でなければ処理
            if (!saveQueue.empty()) {
                // 先頭の保存タスクを取得
                SaveItem item = saveQueue.front();
                saveQueue.pop();
                lock.unlock(); // 処理中はロックを解放
                
                // 画像を回転（スレッド内で実行）
                ofPixels rotatedPixels = rotatePixelsByDegrees(item.pixels, item.rotationDegrees);
                
                // 回転した画像を保存
                ofSaveImage(rotatedPixels, item.fileName);
                
                ofLogNotice() << "Saved rotated image: " << item.fileName;
            } else {
                lock.unlock();
                // キューが空ならスリープしてCPU使用率を下げる
                sleep(10);
            }
        }
    }
    
    std::queue<SaveItem> saveQueue;
    std::mutex mutex;
};
