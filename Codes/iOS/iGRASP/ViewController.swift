//
//  ViewController.swift
//  iGRASP
//
//  Created by Arvin on 2022-01-22.
//

import UIKit
import CoreBluetooth
import Combine



class ViewController: UIViewController{
    
    
    
    var centralManager: CBCentralManager!
    
    var adcCharacteristic: CBCharacteristic!
    var bnoCharacteristic: CBCharacteristic!
    var calibrationCharacteristic: CBCharacteristic!
    var batteryCharacteristic: CBCharacteristic!
    
    var adcCBUUID: CBUUID!
    var bnoCBUUID: CBUUID!
    var calibrationCBUUID: CBUUID!
    var batteryCBUUID: CBUUID!
    
    
    var firttime_right = 0
    var firttime_left = 0
    var pressed = 0
    

    
    var index_init_right = 0.0
    var middle_init_right = 0.0
    var ring_init_right = 0.0
    var pinky_init_right = 0.0
    var thumbs_init_right = 0.0
    
    
    var index_right = 0.0
    var middle_right = 0.0
    var ring_right = 0.0
    var pinky_right = 0.0
    var thumbs_right = 0.0
    
    

    
    
    var index_init_left = 0.0
    var middle_init_left = 0.0
    var ring_init_left = 0.0
    var pinky_init_left = 0.0
    var thumbs_init_left = 0.0
    
    
    var index_left = 0.0
    var middle_left = 0.0
    var ring_left = 0.0
    var pinky_left = 0.0
    var thumbs_left = 0.0
    
    var NUM_DEVICES = 2
    var userDevices: [String: String] = ["EC4DA42ADC7D":"Right","F639DA0C882C":"Left"]
    let kneeBraceUUID = CBUUID(string: "D3091400-B0F3-11EA-B3DE-0242AC130004")
    var UUIDByIdentifier: [String: String] = [:]
    var devices = [CBPeripheral]()
    var connectedCount: Int = 0
    var cancellable: AnyCancellable?
    
    @IBOutlet weak var point: UIImageView!
    // Note: connectionNotification is not sent until characteristics are discovered
    let connectNotification = Notification.Name("connectNotification")
    let disconnectNotification = Notification.Name("disconnectNotification")
    let adcNotification = Notification.Name("adcNotification")
    let bnoNotification = Notification.Name("bnoNotification")
    let calibrationNotification = Notification.Name("calibrationNotification")
    let batteryNotification = Notification.Name("batteryNotification")
    
    
    var macByUUID: [String: String] = [:]
    let MANUFACTURER_LENGTH_ID_BYTES = 2
    //TODO: move to constants
    enum side: String {
        case left = "Left"
        case right = "Right"
    }
    
    
    var isFirst = true
    var isFirsty = true
    var flagleft = false
    var flagright = false
    
    var ismusclefirst = true
    var leftmuscleinit = Array(repeating: 0.0, count: 32)
    var rightmuscleinit = Array(repeating: 0.0, count: 32)
    
    var leftmuscleprev = Array(repeating: 0.0, count: 32)
    var rightmuscleprev = Array(repeating: 0.0, count: 32)
    
    var leftmusclenew = Array(repeating: 0.0, count: 32)
    var rightmusclenew = Array(repeating: 0.0, count: 32)
    
    
    var differenceRight = 0.0
    var differenceLeft = 0.0
    var leftAngle = 0.0
    var rightAngle = 0.0
    
    var leftInitQuadQuat = Quaternion()
    var leftInitCalfQuat = Quaternion()
    var rightInitQuadQuat = Quaternion()
    var rightInitCalfQuat = Quaternion()
    var InitQuadQuat = Quaternion()
    var InitCalfQuat = Quaternion()
    
    var leftInitQuadAcc = ThreeDimension()
    var leftInitCalfAcc = ThreeDimension()
    var rightInitQuadAcc = ThreeDimension()
    var rightInitCalfAcc = ThreeDimension()
    
    var leftQuadAcc = ThreeDimension()
    var leftCalfAcc = ThreeDimension()
    var rightQuadAcc = ThreeDimension()
    var rightCalfAcc = ThreeDimension()
    
    var leftQuadAccPrev = ThreeDimension()
    var leftCalfAccPrev = ThreeDimension()
    var rightQuadAccPrev = ThreeDimension()
    var rightCalfAccPrev = ThreeDimension()
    
    
    var leftQuadFilt = ThreeDimension()
    var leftCalfFilt = ThreeDimension()
    var rightQuadFilt = ThreeDimension()
    var rightCalfFilt = ThreeDimension()
    
    var leftQuadFiltPrev = ThreeDimension()
    var leftCalfFiltPrev = ThreeDimension()
    var rightQuadFiltPrev = ThreeDimension()
    var rightCalfFiltPrev = ThreeDimension()
    
    var AngleX = 0.0
    var AngleY = 0.0
    var AngleZ = 0.0
    
        
    var mse: [Double] = [0,0,0,0]
    
    struct Quaternion {
        var w, x, y, z: Double
        init() {
            w = 0; x = 0; y = 0; z = 0
        }
        init(w: Double, x: Double, y: Double, z: Double) {
            self.w = w
            self.x = x
            self.y = y
            self.z = z
        }
    }
    
    struct ThreeDimension {
        var x, y, z: Double
        init(){
            x = 0; y = 0; z = 0
        }
        init(x: Double, y: Double, z: Double){
            self.x = x
            self.y = y
            self.z = z
        }
    }
    func removeNoise (a: ThreeDimension, thr: Double) -> ThreeDimension{
        var results = a
        
        if results.x < thr{
            results.x = 0
        }
        if results.y < thr{
            results.y = 0
        }
        if results.z < thr{
            results.z = 0
        }
        return results
    }
    
    func addThreeDimension (a: ThreeDimension, b: ThreeDimension) -> ThreeDimension{
        var results = ThreeDimension(x: a.x+b.x, y: a.y+b.y, z: a.z+b.z)
        return results
    }
    
    func minusThreeDimension (a: ThreeDimension, b: ThreeDimension) -> ThreeDimension{
        var results = ThreeDimension(x: a.x-b.x, y: a.y-b.y, z: a.z-b.z)
        return results
    }
    
    func multiThreeDimension (a: Double, b: ThreeDimension) -> ThreeDimension{
        var results = ThreeDimension(x: a * b.x, y: a * b.y, z: a * b.z)
        return results
    }
    
    
    // Todo:
    func quaternionRotate (a: Quaternion, b: ThreeDimension) -> ThreeDimension{
        var results = ThreeDimension()
        results.x = (1-2*pow(a.y,2)-2*pow(a.z,2))*b.x+2*(a.x*a.y+a.w*a.z)*b.y+2*(a.x*a.z-a.w*a.y)*b.z
        results.y = 2*(a.x*a.y-a.w*a.z)*b.x + (1-2*pow(a.x,2)-2*pow(a.z,2))*b.y+2*(a.y*a.z+a.w*a.x)*b.z
        results.z = 2*(a.x*a.z+a.w*a.y)*b.x+2*(a.y*a.z - a.w*a.x)*b.y+(1-2*pow(a.x,2)-2*pow(a.y,2))*b.z
        return results
    }
    
    struct Filter{
        let b0, b1, a: Double
        init(){
            b0 = 0; b1 = 0; a = 0
        }
        init(b0: Double, b1: Double, a: Double){
            self.b0 = b0
            self.b1 = b1
            self.a = a
        }
    }
    
    func quaternionConjugate(_ q: Quaternion) -> Quaternion {
        return Quaternion(w: q.w, x: -q.x, y: -q.y, z: -q.z)
    }
    
    func quaternionMultiply(q1: Quaternion, q2: Quaternion) -> Quaternion {
        return Quaternion(w: -q2.x * q1.x - q2.y * q1.y - q2.z * q1.z + q2.w * q1.w,
                          x: q2.x * q1.w + q2.y * q1.z - q2.z * q1.y + q2.w * q1.x,
                          y: -q2.x * q1.z + q2.y * q1.w + q2.z * q1.x + q2.w * q1.y,
                          z: q2.x * q1.y - q2.y * q1.x + q2.z * q1.w + q2.w * q1.z)
    }
    
    struct EulerAngle {
        let roll, pitch, yaw: Double
    }
    
    //Note result is in radians
    func quaternionToEuler(_ q: Quaternion) -> EulerAngle {
        // Reference: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
        
        let sinr_cosp = 2 * (q.w * q.x + q.y * q.z)
        let cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y)
        let roll = atan2(sinr_cosp, cosr_cosp)
        
        let sinp = 2 * (q.w * q.y - q.z * q.x)
        let pitch = abs(sinp) >= 1 ? copysign(Double.pi / 2, sinp) : asin(sinp)
        
        let siny_cosp = 2 * (q.w * q.z + q.x * q.y)
        let cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z)
        let yaw = atan2(siny_cosp, cosy_cosp)
        
        return EulerAngle(roll: roll * 180 / Double.pi,
                          pitch: pitch * 180 / Double.pi,
                          yaw: yaw * 180 / Double.pi)
    }

    func calcMuscleActivation(leftAdc: [Double], rightAdc: [Double]) -> [Double] {
        //Muscle Order: leftQuad, leftHamstring, leftCalf, leftShin, rightQuad, rightHamstring, rightCalf, rightShin
        
        if ismusclefirst == true{
            leftmuscleinit = leftAdc
            rightmuscleinit = rightAdc
            leftmuscleprev = leftAdc
            rightmuscleprev = rightAdc
            ismusclefirst = false
        }
        leftmusclenew = leftAdc
        rightmusclenew = rightAdc
        
        for i in 0...31{
            if abs(leftmusclenew[i]-leftmuscleprev[i]) > 200 {
                leftmusclenew[i] = leftmuscleinit[i]
            }
            else{
                leftmuscleprev[i] = leftAdc[i]
            }
            if abs(rightmusclenew[i]-rightmuscleprev[i]) > 200 {
                rightmusclenew[i] = rightmuscleinit[i]
            }
            else{
                rightmuscleprev[i] = rightAdc[i]
            }
        }
       
        
        var muscleActivation = Array(repeating: 0.0, count: 8)
            muscleActivation[0] = 100 * (leftmusclenew[22]/leftmuscleinit[22] + leftmusclenew[23]/leftmuscleinit[23] + leftmusclenew[10]/leftmuscleinit[10] + leftmusclenew[11]/leftmuscleinit[11] - 4)/4
            //muscleActivation[1] = leftAdc[12...15].reduce(0, +)
            muscleActivation[1] = 100 * (leftmusclenew[12]/leftmuscleinit[12] + leftmusclenew[13]/leftmuscleinit[13] + leftmusclenew[14]/leftmuscleinit[14] + leftmusclenew[15]/leftmuscleinit[15] - 4)/4
            //muscleActivation[2] = leftAdc[1...4].reduce(0, +)
            muscleActivation[2] = 100 * (leftmusclenew[1]/leftmuscleinit[1] + leftmusclenew[2]/leftmuscleinit[2] + leftmusclenew[3]/leftmuscleinit[3] + leftmusclenew[4]/leftmuscleinit[4] - 4)/4
            muscleActivation[3] = 100 * (leftmusclenew[0]/leftmuscleinit[0] + leftmusclenew[5]/leftmuscleinit[5] - 2)/2
            muscleActivation[4] = 100 * (rightmusclenew[22]/rightmuscleinit[22] + rightmusclenew[23]/rightmuscleinit[23] + rightmusclenew[10]/rightmuscleinit[10] + rightmusclenew[11]/rightmuscleinit[11] - 4)/4
            muscleActivation[5] = 100 * (rightmusclenew[12]/rightmuscleinit[12] + rightmusclenew[13]/rightmuscleinit[13] + rightmusclenew[14]/rightmuscleinit[14] + rightmusclenew[15]/rightmuscleinit[15] - 4 )/4
            muscleActivation[6] = 100 * (rightmusclenew[1]/rightmuscleinit[1] + rightmusclenew[2]/rightmuscleinit[2] + rightmusclenew[3]/rightmuscleinit[3] + rightmusclenew[4]/rightmuscleinit[4] - 4)/4
            muscleActivation[7] = 100 * (rightmusclenew[0]/rightmuscleinit[0] + rightmusclenew[5]/rightmuscleinit[5] - 2)/2
            return muscleActivation
    }
    
    func calcCenterOfMass(leftBno: [Double], rightBno: [Double]) -> (Double, Double) {
        return (0, 0)
    }
    
    
    
    func calcAngle(leftBno: [Double], rightBno: [Double]) -> (Double, Double) {
        var leftQuadQuat, leftCalfQuat, rightQuadQuat, rightCalfQuat, leftInnerProd, rightInnerProd: Quaternion
        leftQuadQuat = Quaternion(w: leftBno[3] / pow(2.0, 14), x: -leftBno[4] / pow(2.0, 14),
                                 y: -leftBno[5] / pow(2.0, 14), z: -leftBno[6] / pow(2.0, 14))
        leftCalfQuat = Quaternion(w: leftBno[13] / pow(2.0, 14), x: -leftBno[14] / pow(2.0, 14),
                                 y: -leftBno[15] / pow(2.0, 14), z: -leftBno[16] / pow(2.0, 14))
        rightQuadQuat = Quaternion(w: rightBno[3] / pow(2.0, 14), x: -rightBno[4] / pow(2.0, 14),
                                  y: -rightBno[5] / pow(2.0, 14), z: -rightBno[6] / pow(2.0, 14))
        rightCalfQuat = Quaternion(w: rightBno[13] / pow(2.0, 14), x: -rightBno[14] / pow(2.0, 14),
                                  y: -rightBno[15] / pow(2.0, 14), z: -rightBno[16] / pow(2.0, 14))
        if isFirst {
            leftInitQuadQuat  = leftQuadQuat
            leftInitCalfQuat  = leftCalfQuat
            rightInitQuadQuat = rightQuadQuat
            rightInitCalfQuat = rightCalfQuat
            isFirst = false
        }
        
        leftQuadQuat = quaternionMultiply(q1: leftQuadQuat, q2: quaternionConjugate(leftInitQuadQuat))
        leftCalfQuat = quaternionMultiply(q1: leftCalfQuat, q2: quaternionConjugate(leftInitCalfQuat))
        rightQuadQuat = quaternionMultiply(q1: rightQuadQuat, q2: quaternionConjugate(rightInitQuadQuat))
        rightCalfQuat = quaternionMultiply(q1: rightCalfQuat, q2: quaternionConjugate(rightInitCalfQuat))
        
        leftInnerProd = quaternionMultiply(q1: leftQuadQuat, q2: quaternionConjugate(leftCalfQuat))
        rightInnerProd = quaternionMultiply(q1: rightQuadQuat, q2: quaternionConjugate(rightCalfQuat))
        
        let leftBoundedW = max(min(1, leftInnerProd.y), -1)
        let leftTheta = 2 * acos(leftBoundedW) * 180/Double.pi-173
        
        let rightBoundedW = max(min(1, rightInnerProd.y), -1)
        let rightTheta = 2 * acos(rightBoundedW) * 180/Double.pi-173
        
        return (leftTheta, rightTheta)
        
    }
    
    
    
    func calcAngleToSave(Bno: [Double]) -> (Double, Double, Double) {
        var QuadQuat, CalfQuat, InnerProd: Quaternion
        QuadQuat = Quaternion(w: Bno[3] / pow(2.0, 14), x: -Bno[4] / pow(2.0, 14),
                                 y: -Bno[5] / pow(2.0, 14), z: -Bno[6] / pow(2.0, 14))
        CalfQuat = Quaternion(w: Bno[13] / pow(2.0, 14), x: -Bno[14] / pow(2.0, 14),
                                 y: -Bno[15] / pow(2.0, 14), z: -Bno[16] / pow(2.0, 14))

        if isFirsty {
            InitQuadQuat  = QuadQuat
            InitCalfQuat  = CalfQuat
            isFirsty = false
        }
        
        QuadQuat = quaternionMultiply(q1: QuadQuat, q2: quaternionConjugate(InitQuadQuat))
        CalfQuat = quaternionMultiply(q1: CalfQuat, q2: quaternionConjugate(InitCalfQuat))

        
        InnerProd = quaternionMultiply(q1: QuadQuat, q2: quaternionConjugate(CalfQuat))

        
        let BoundedX = max(min(1, InnerProd.x), -1)
        let ThetaX = 2 * acos(BoundedX) * 180/Double.pi-173
        
        let BoundedY = max(min(1, InnerProd.y), -1)
        let ThetaY = 2 * acos(BoundedY) * 180/Double.pi-173
        
        let BoundedZ = max(min(1, InnerProd.z), -1)
        let ThetaZ = 2 * acos(BoundedZ) * 180/Double.pi-173

        
        return (ThetaX, ThetaY, ThetaZ)
        
    }
    
    func calcAngle_jade(leftBno: [Double], rightBno: [Double]) -> (Double, Double) {
        var leftQuadQuat, leftCalfQuat, rightQuadQuat, rightCalfQuat, leftInnerProd, rightInnerProd: Quaternion
        leftQuadQuat = Quaternion(w: leftBno[3] / pow(2.0, 14), x: -leftBno[4] / pow(2.0, 14),
                                 y: -leftBno[5] / pow(2.0, 14), z: -leftBno[6] / pow(2.0, 14))
        leftCalfQuat = Quaternion(w: leftBno[13] / pow(2.0, 14), x: -leftBno[14] / pow(2.0, 14),
                                 y: -leftBno[15] / pow(2.0, 14), z: -leftBno[16] / pow(2.0, 14))
        rightQuadQuat = Quaternion(w: rightBno[3] / pow(2.0, 14), x: -rightBno[4] / pow(2.0, 14),
                                  y: -rightBno[5] / pow(2.0, 14), z: -rightBno[6] / pow(2.0, 14))
        rightCalfQuat = Quaternion(w: rightBno[13] / pow(2.0, 14), x: -rightBno[14] / pow(2.0, 14),
                                  y: -rightBno[15] / pow(2.0, 14), z: -rightBno[16] / pow(2.0, 14))
        if isFirst {
            leftInitQuadQuat  = leftQuadQuat
            leftInitCalfQuat  = leftCalfQuat
            rightInitQuadQuat = rightQuadQuat
            rightInitCalfQuat = rightCalfQuat
            isFirst = false
        }
        
        leftQuadQuat = quaternionMultiply(q1: leftQuadQuat, q2: quaternionConjugate(leftInitQuadQuat))
        leftCalfQuat = quaternionMultiply(q1: leftCalfQuat, q2: quaternionConjugate(leftInitCalfQuat))
        rightQuadQuat = quaternionMultiply(q1: rightQuadQuat, q2: quaternionConjugate(rightInitQuadQuat))
        rightCalfQuat = quaternionMultiply(q1: rightCalfQuat, q2: quaternionConjugate(rightInitCalfQuat))
        
        leftInnerProd = quaternionMultiply(q1: leftQuadQuat, q2: quaternionConjugate(leftCalfQuat))
        rightInnerProd = quaternionMultiply(q1: rightQuadQuat, q2: quaternionConjugate(rightCalfQuat))
        
        let leftBoundedW = max(min(1, leftInnerProd.w), -1)
        let leftTheta = 2 * acos(leftBoundedW) * 180/Double.pi
        
        let rightBoundedW = max(min(1, rightInnerProd.w), -1)
        let rightTheta = 2 * acos(rightBoundedW) * 180/Double.pi
        
        // get acceleration values:   acc/10
        
        var leftQuadAcc, leftCalfAcc, rightQuadAcc, rightCalfAcc : ThreeDimension
        
        leftQuadAcc = ThreeDimension(x: leftBno[0]/10,y: leftBno[1]/10,z: leftBno[2]/10)
        leftCalfAcc = ThreeDimension(x: leftBno[10]/10,y: leftBno[11]/10,z: leftBno[12]/10)
        rightQuadAcc = ThreeDimension(x: rightBno[0]/10,y: rightBno[1]/10,z: rightBno[2]/10)
        rightCalfAcc = ThreeDimension(x: rightBno[10]/10,y: rightBno[11]/10,z: rightBno[12]/10)
        
        // define b, a
        let filter = Filter(b0: 0.1246, b1: 0.1246, a: -0.7508)
        
        
        //filter data
        leftQuadFilt = minusThreeDimension(a: addThreeDimension(a: multiThreeDimension(a: filter.b0, b: leftQuadAcc), b: multiThreeDimension(a: filter.b1, b: leftQuadAccPrev)), b: multiThreeDimension(a: filter.a, b: leftQuadFiltPrev))
        
        leftCalfFilt = minusThreeDimension(a: addThreeDimension(a: multiThreeDimension(a: filter.b0, b: leftCalfAcc), b: multiThreeDimension(a: filter.b1, b: leftCalfAccPrev)), b: multiThreeDimension(a: filter.a, b: leftCalfFiltPrev))
        
        rightQuadFilt = minusThreeDimension(a: addThreeDimension(a: multiThreeDimension(a: filter.b0, b: rightQuadAcc), b: multiThreeDimension(a: filter.b1, b: rightQuadAccPrev)), b: multiThreeDimension(a: filter.a, b: rightQuadFiltPrev))
        
        rightCalfFilt = minusThreeDimension(a: addThreeDimension(a: multiThreeDimension(a: filter.b0, b: rightCalfAcc), b: multiThreeDimension(a: filter.b1, b: rightCalfAccPrev)), b: multiThreeDimension(a: filter.a, b: rightCalfFiltPrev))
        
                                                                                                            
        //quaternionRotate
        leftQuadFilt = quaternionRotate(a: quaternionConjugate(leftQuadQuat),b: leftQuadFilt)
        leftCalfFilt = quaternionRotate(a: quaternionConjugate(leftCalfQuat),b: leftCalfFilt)
        rightQuadFilt = quaternionRotate(a: quaternionConjugate(rightQuadQuat),b: rightQuadFilt)
        rightCalfFilt = quaternionRotate(a: quaternionConjugate(rightCalfQuat),b: rightCalfFilt)
        
        
        
        // is first
        if isFirst{
        leftInitQuadAcc = leftQuadFilt
        leftInitCalfAcc = leftCalfFilt
        rightInitQuadAcc = rightQuadFilt
        rightInitCalfAcc = rightCalfFilt
        isFirst = false
        }
        
        // removing the ofset
        leftQuadFilt = minusThreeDimension(a: leftQuadFilt, b: leftInitQuadAcc)
        leftCalfFilt = minusThreeDimension(a: leftCalfFilt, b: leftInitCalfAcc)
        

            
        // where j is columns remove the noise
        
        leftQuadFilt = removeNoise(a: leftQuadFilt, thr: 0.5)
        leftCalfFilt = removeNoise(a: leftCalfFilt, thr: 0.1)
        rightQuadFilt = removeNoise(a: rightQuadFilt, thr: 0.5)
        rightCalfFilt = removeNoise(a: rightCalfFilt, thr: 0.1)
        

        
        var leftQuadTotalAcc = sqrt(pow(leftQuadFilt.x,2)+pow(leftQuadFilt.y,2)+pow(leftQuadFilt.z,2
        ))
        var leftCalfTotalAcc = sqrt(pow(leftCalfFilt.x,2)+pow(leftCalfFilt.y,2)+pow(leftCalfFilt.z,2))
        var rightQuadTotalAcc = sqrt(pow(rightQuadFilt.x,2)+pow(rightQuadFilt.y,2)+pow(rightQuadFilt.z,2))
        var rightCalfTotalAcc = sqrt(pow(rightCalfFilt.x,2)+pow(rightCalfFilt.y,2)+pow(rightCalfFilt.z,2))
        
        var leftTotalAcc = sqrt(pow(leftQuadTotalAcc,2) + pow(leftCalfTotalAcc,2))
        var rightTotalAcc = sqrt(pow(rightQuadTotalAcc,2) + pow(rightCalfTotalAcc,2))
        
        if leftTotalAcc == 0 {
            if flagleft == false {
                flagleft = true
                differenceLeft = leftTheta
                leftAngle = 0
            }
            else{
                //difference = leftTheta
                leftAngle = 0
                }
            }
        else {
            flagleft = false
            leftAngle = leftTheta + abs(differenceLeft)
    }
        if rightTotalAcc == 0 {
            if flagright == false {
                flagright = true
                differenceRight = rightTheta
                rightAngle = 0
            }
            else{
                //difference = leftTheta
                rightAngle = 0
                }
            }
        else {
            flagright = false
            rightAngle = rightTheta + abs(differenceRight)
    }
            
        
   return (leftTheta, rightTheta)
        
}
        
    
    func calcRep(leftBno: [Double], rightBno: [Double], angleThreshold: Double, hasLeftAndRight: Bool?) -> Bool {
        guard let hasLeftAndRight = hasLeftAndRight else { return false}
        let (leftAngle, rightAngle) = calcAngle(leftBno: leftBno, rightBno: rightBno)
        if mse.count > 3 {
            mse.remove(at: 0)
        }
        if hasLeftAndRight, leftAngle > angleThreshold,
            rightAngle > angleThreshold {
            mse.append(1)
        } else if !hasLeftAndRight, (leftAngle > angleThreshold || rightAngle > angleThreshold) {
            mse.append(1)
        } else {
            mse.append(0)
        }
        if (mse[0] == 0) && (mse[1] == 0) && (mse[2] == 1) && (mse[3] == 1) {
            return true
        } else {
            return false
        }
    }
    
    @IBOutlet weak var text: UILabel!
    override func viewDidLoad() {
            super.viewDidLoad()
            centralManager = CBCentralManager(delegate: self, queue: nil)
            print("Scanning for devices")
            NotificationCenter.default.addObserver(self, selector: #selector(receivedADC), name: adcNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(receivedBNO), name: bnoNotification, object: nil)

        }

    @objc func receivedBNO(_ notification: Notification){
        guard let uuid  = notification.userInfo!["uuid"] as? String else {return}
        guard let data = notification.userInfo!["data"] as? [Double] else {return}
        guard let timestamp = notification.userInfo!["timestamp"] as? Date else {return}
        (AngleX, AngleY, AngleZ) = calcAngleToSave(Bno: data)
        //print(AngleX, AngleY, AngleZ)
        point.transform.ty = CGFloat(-7*AngleX);
        point.transform.tx = CGFloat(-7*AngleZ);
//        if sensorDataByDevice[uuid]!.count > 32 {
//            let adcData = Array(sensorDataByDevice[uuid]![..<32])
//            var bnoData = [Double]()
//            if sensorDataByDevice[uuid]!.count > 43 {
//                bnoData = Array(sensorDataByDevice[uuid]![32..<53])
//            } else {
//                //Note for the old peripherals 32..<42 is 1 bno and 43 is ecg(unused)
//                bnoData = Array(sensorDataByDevice[uuid]![32..<42])
//            }
//            let (AngleX, AngleY, AngleZ) = calcAngleToSave(Bno: bnoData)
//            print(AngleX, AngleY, AngleZ)
//
//        }

            }
    
    //Updates the progress bars based on values returned by the knee processor

    
    
        @objc func receivedADC(_ notification: Notification){
        guard let uuid  = notification.userInfo!["uuid"] as? String else {return}
        guard let data = notification.userInfo!["data"] as? [Double] else {return}
        
       
        if uuid == "EC4DA42ADC7D"{
            if firttime_right == 0{
                firttime_right = 1
                index_init_right = data[4]
                thumbs_init_right = data[31]
                pinky_init_right = data[12]
                ring_init_right = data[23]
                middle_init_right = data[28]

            }
            else{
                index_right = 100*(data[4]/index_init_right-1)
                thumbs_right = 100*(data[31]/thumbs_init_right-1)
                middle_right = 100*(data[28]/middle_init_right-1)
                ring_right = 100*(data[23]/ring_init_right-1)
                pinky_right = 100*(data[12]/pinky_init_right-1)
                //print(index_right,thumbs_right,middle_right,ring_right,pinky_right)

            }
        }
        else{

            if firttime_left == 0{
                firttime_left = 1
                index_init_left = data[4]
                thumbs_init_left = data[31]
                pinky_init_left = data[12]
                ring_init_left = data[23]
                middle_init_left = data[28]

            }
            else{
                index_left = 100*(data[4]/index_init_left-1)
                thumbs_left = 100*(data[31]/thumbs_init_left-1)
                middle_left = 100*(data[28]/middle_init_left-1)
                ring_left = 100*(data[23]/ring_init_left-1)
                pinky_left = 100*(data[12]/pinky_init_left-1)
                //print(index_left,thumbs_left,middle_left,ring_left,pinky_left)
            }
        }
        
            if (index_right>20) || (index_left>20)   {
                point.tintColor = .systemPurple
                var dotPath = UIBezierPath(ovalIn: CGRect(x: point.center.x-CGFloat(7*AngleZ), y: point.center.y-CGFloat(7*AngleX), width: 30, height: 30))
                print(AngleX,AngleZ)

                let layer = CAShapeLayer()
                layer.path = dotPath.cgPath
                layer.fillColor = UIColor.purple.cgColor

                self.view.layer.addSublayer(layer)

            }
            else if (middle_right>20) || (middle_left>20){
                point.tintColor = .systemRed
                var dotPath = UIBezierPath(ovalIn: CGRect(x: point.center.x-CGFloat(7*AngleZ), y: point.center.y-CGFloat(7*AngleX), width: 30, height: 30))
                print(AngleX,AngleZ)

                let layer = CAShapeLayer()
                layer.path = dotPath.cgPath
                layer.fillColor = UIColor.red.cgColor

                self.view.layer.addSublayer(layer)

                self.view.layer.addSublayer(layer)
            }
            else if (ring_right>20) || (ring_left>20){
                point.tintColor = .systemGreen
                var dotPath = UIBezierPath(ovalIn: CGRect(x: point.center.x-CGFloat(7*AngleZ), y: point.center.y-CGFloat(7*AngleX), width: 30, height: 30))
                print(AngleX,AngleZ)

                let layer = CAShapeLayer()
                layer.path = dotPath.cgPath
                layer.fillColor = UIColor.green.cgColor

                self.view.layer.addSublayer(layer)
            }
            else if (pinky_right>20) || (pinky_left>20){
                point.tintColor = .systemBlue
                var dotPath = UIBezierPath(ovalIn: CGRect(x: point.center.x-CGFloat(7*AngleZ), y: point.center.y-CGFloat(7*AngleX), width: 30, height: 30))
                print(AngleX,AngleZ)

                let layer = CAShapeLayer()
                layer.path = dotPath.cgPath
                layer.fillColor = UIColor.blue.cgColor

                self.view.layer.addSublayer(layer)
            }
            else{
                point.tintColor = .systemGray
                
             

                
            }
            
            
        }
    
    
    
    
    }



extension ViewController: CBCentralManagerDelegate{
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state{
        case .unknown:
            print("central.state is unknown")
        case .resetting:
            print("central.state is resetting")
        case .unsupported:
            print("central.state is unsupported")
        case .unauthorized:
            print("central.state is unauthorized")
        case .poweredOff:
            print("central.state is poweredOff")
        case .poweredOn:
            print("central.state is poweredOn")
            // Get user devices registered to the current user
            self.centralManager.scanForPeripherals(withServices: [self.kneeBraceUUID])

        @unknown default:
            fatalError()
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        // Peripheral devices are guaranteed to belong to the current user so we can safely attempt to connect to them
        if let manuData = advertisementData["kCBAdvDataManufacturerData"] as? Data {
            let macAddr = Array(manuData[MANUFACTURER_LENGTH_ID_BYTES...]).map { String(format: "%02hhX", $0) }.joined()
            if userDevices[macAddr] != nil {
                // we keep a reference to the peripheral
                macByUUID[peripheral.identifier.uuidString] = macAddr
                peripheral.delegate = self
                devices.append(peripheral)
                centralManager.connect(peripheral)

                NotificationCenter.default.post(name: connectNotification, object: nil, userInfo: [
                    "uuid": macAddr,
                    "name": peripheral.name ?? ""
                ])
            }
            if userDevices[macAddr] == side.left.rawValue {

            } else if userDevices[macAddr] == side.right.rawValue {

            } else {
                //TODO: you do not own this device
            }
        }
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("Peripheral connected !")

        connectedCount += 1

        peripheral.discoverServices(nil)
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("peripheral disconnected \(error)")



        connectedCount -= 1
        devices.removeAll(where: {$0 == peripheral})

        centralManager.scanForPeripherals(withServices: [kneeBraceUUID])
        NotificationCenter.default.post(name: disconnectNotification, object: nil, userInfo: [
            "uuid": macByUUID[peripheral.identifier.uuidString]!,
            "name": peripheral.name!
        ])
    }


}

extension ViewController: CBPeripheralDelegate{

    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        print("Set for discovery ")
        guard let services = peripheral.services else { return }

        let texavieService: CBService = services[0]
        peripheral.discoverCharacteristics(nil, for: texavieService)
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }
        print("Characteristics of peripheral \(macByUUID[peripheral.identifier.uuidString]!): \(characteristics)")
        adcCharacteristic = characteristics[0]
        bnoCharacteristic = characteristics[2]
        calibrationCharacteristic = characteristics[3]
        batteryCharacteristic = characteristics[1]

        adcCBUUID = adcCharacteristic.uuid
        bnoCBUUID = bnoCharacteristic.uuid
        calibrationCBUUID = calibrationCharacteristic.uuid
        batteryCBUUID = batteryCharacteristic.uuid

        peripheral.setNotifyValue(true, for: adcCharacteristic)
        peripheral.setNotifyValue(true, for: bnoCharacteristic)
        //peripheral.setNotifyValue(true, for: calibrationCharacteristic)
        //peripheral.setNotifyValue(true, for: batteryCharacteristic)
        if connectedCount >= NUM_DEVICES {
            centralManager.stopScan()

        }
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        var adcData = [Double]()
        var bnoData = [Double]()
        var calibrationData: (Int, Int, Int, Int, Int)
        var batteryStatus: Int

        let date = Date()
        let formatter = DateFormatter()
        formatter.timeZone = .current
        formatter.dateFormat = "HH:mm:ss.SSSS"
        let ts = date

        switch characteristic.uuid {
        case adcCBUUID:
            adcData = decodeADC(char: characteristic)
            NotificationCenter.default.post(name: adcNotification, object: nil, userInfo: ["uuid": macByUUID[peripheral.identifier.uuidString]!, "data": adcData, "timestamp": ts, "name": peripheral.name!])
        case bnoCBUUID:
             bnoData = decodeBNO(char: characteristic)
             NotificationCenter.default.post(name: bnoNotification, object: nil, userInfo: ["uuid": macByUUID[peripheral.identifier.uuidString]!, "data": bnoData, "timestamp": ts, "name": peripheral.name!])
        case calibrationCBUUID:
            calibrationData = decodeCalibration(char: characteristic)
            NotificationCenter.default.post(name: calibrationNotification, object: nil, userInfo: ["uuid": macByUUID[peripheral.identifier.uuidString]!, "data": calibrationData, "timestamp": ts, "name": peripheral.name!])
        case batteryCBUUID:
            batteryStatus = decodeBatteryStatus(char: characteristic)
            NotificationCenter.default.post(name: batteryNotification, object: nil, userInfo: ["uuid": macByUUID[peripheral.identifier.uuidString]!, "status": batteryStatus, "timestamp": ts, "name": peripheral.name!])
        default:
            print("Unknown Characteristic UUID")
        }
    }

    func decodeADC(char: CBCharacteristic) -> [Double] {
        guard let charData = char.value else { return [] }

        let data = [UInt8](charData)
        var doubleArray = [Double]()
        for i in stride(from: 0, through: data.count - 1, by: 2) {
            let LSB = data[i]
            let MSB = data[i + 1]

            let val16 = UInt16(MSB) << 8 | UInt16(LSB)

            doubleArray.append(Double(val16) / 10.0)
        }
        // Please remove this at some point in the future
        let pinOrder = [3: 0, 7: 1, 11: 2, 15: 3, 31: 4,
                        27: 5, 23: 6, 19: 7, 2: 8, 6: 9,
                        10: 10, 14: 11, 30: 12, 26: 13, 22: 14,
                        18: 15, 16: 16, 20: 17, 24: 18, 28: 19,
                        12: 20, 8: 21, 4: 22, 0: 23, 17: 24,
                        21: 25, 25: 26, 29: 27, 13: 28, 9: 29,
                        5: 30, 1: 31]
        var correctOrdered = Array(repeating: 0.0, count: 32)
        for pin in 0..<pinOrder.count {
            correctOrdered[pinOrder[pin]!] = doubleArray[pin]
        }
        return correctOrdered
    }

    func decodeBNO(char: CBCharacteristic) -> [Double] {
        guard let charData = char.value else { return [] }

        let data = [UInt8](charData)

        var doubleArray = [Double]()
        for i in stride(from: 0, through: data.count - 1, by: 2){
            let LSB = data[i]
            let MSB = data[i + 1]

            let val16 = Int16(MSB) << 8 | Int16(LSB)

            doubleArray.append(Double(val16))
        }
        return doubleArray
    }

    func decodeCalibration(char: CBCharacteristic) -> (Int, Int, Int, Int, Int) {
        guard let charData = char.value else { return (0, 0, 0, 0, 0) }

        let data = [UInt8](charData)

        var calibrationData: (Int, Int, Int, Int, Int) = (0, 0, 0, 0, 0)
        for i in stride(from: 0, through: data.count - 1, by: 5) {
            let CS = Int(data[i])     // System
            let CA = Int(data[i + 1]) // Accelerometer
            let CG = Int(data[i + 2]) // Gyroscope
            let CM = Int(data[i + 3]) // Magnetometer
            let isCalibrated = Int(data[i + 4])
            calibrationData = (CS, CA, CG, CM, isCalibrated)
        }
        return calibrationData
    }

    func decodeBatteryStatus(char: CBCharacteristic) -> Int {
        guard let data = char.value else { return -1 }

        let voltageData = data.withUnsafeBytes { $0.load(as: Float.self) }
        // Placeholder Data source: https://www.powerstream.com/lithium-ion-charge-voltage.htm
        let percentageByVoltage: [Float: Int] = [4.2: 100, 4.06: 90, 3.98: 80, 3.92: 70, 3.87: 60, 3.82: 50, 3.79: 40, 3.77: 30, 3.74: 20, 3.68: 10, 3.45: 5, 3.0: 0]
        let voltages: [Float] = [4.2, 4.06, 3.98, 3.92, 3.87, 3.82, 3.79, 3.77, 3.74, 3.68, 3.45, 3.0]
        for voltage in voltages {
            if voltageData > voltage {
                print(percentageByVoltage[voltage]!)
                return percentageByVoltage[voltage]!
            }
        }

        return 100
    }

}
