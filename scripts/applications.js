

const fs = require("fs");



// 定义 SegmentTreeNode 类
class SegmentTreeNode {
    constructor(sTime, eTime, level, index, i, min = 0, max = 0, ave = 0, id, 
        minDiff = null, maxDiff = null, aveDiff = null, leftChild = null, rightChild = null, leftIndex = null, rightIndex = null, parent = null, isBuild=false) {
        
        this.sTime = sTime;       // 开始时间
        this.eTime = eTime;       // 结束时间
        this.level = level;       // 层级
        this.index = index;       // 当前节点的索引
        this.i = i;               // 当前节点在该层的第几个位置
        this.min = min;           // 当前节点的最小值
        this.max = max;           // 当前节点的最大值
        this.ave = ave;
        this.sum = ave*(eTime-sTime+1)

        this.id = id;             // 当前节点的唯一ID
        this.minDiff = minDiff;   // min值的差异
        this.maxDiff = maxDiff;   // max值的差异
        this.aveDiff = aveDiff;
        this.leftChild = leftChild;         // 左孩子节点
        this.rightChild = rightChild;       // 右孩子节点
        this.leftIndex = leftIndex;   // 左孩子的索引
        this.rightIndex = rightIndex; // 右孩子的索引
        this.parent = parent; //父亲节点
        this.isBuild = isBuild

        this.isHuisu = false

        //双向链表
        this.preNode = null
        this.nextNode = null

        //nodeCount ++

    }
}


// 定义 SegmentTree 类
class SegmentTree {
    constructor(tableName,columns,index, flagBuffer, maxNodeNum=0) {
        this.root = null;          // 根节点
        this.realDataNum = 100000000
        this.maxNodeNum = 100000000
        this.max_level = 0
        //this.nodes = new Array(maxNodes).fill(null);
        this.nodes = {}
        this.table_name = tableName;   // 数据库中的表名，命令行传入
        this.columns = columns
        this.index = index
        this.flag = flagBuffer;     // 读取的 flag 二进制数组
        this.cache = null

        this.head = null

        this.nodeCount =0
        this.nodeCountDelta =0

        this.belongsToScreen = null
        this.funInfo = null

        this.patentDelete = false
    }

    // 添加节点方法
    addNode(sTime, eTime, level, index, i, min = 0, max = 0, ave = 0, id, 
        minDiff = null, maxDiff = null, aveDiff = null, leftChild = null, rightChild = null, leftIndex = null, rightIndex = null, parent = null, isBuild=false) {
        
        const node = new SegmentTreeNode(sTime, eTime, level, index, i, min, max, ave, id, 
            minDiff, maxDiff, aveDiff, leftChild, rightChild, leftIndex, rightIndex, parent, isBuild);
        
        this.nodeCount ++ 
        this.nodeCountDelta ++

        if (this.root === null) {
            this.root = node;     // 如果根节点为空，则设置为根节点
        }

        //this.nodes[index] = new SegmentTreeNode(sTime, eTime, level, index, i, min, max, id, minDiff, maxDiff, leftChild, rightChild, leftIndex, rightIndex);    // 将节点添加到数组中
        return node;
    }


    buildParent(leftNode, rightNode){
        //不是同一个父亲
        if(leftNode.parent.sTime != rightNode.parent.sTime){
            console.log('不是同一个父亲')
            return null
        }
        
        if(!leftNode.parent.isBuild){
            leftNode.parent.isBuild = true
            this.nodeCount ++ 
            this.nodeCountDelta ++
        }

        return leftNode.parent
    }

    deleteNode(node){
        if(node.isBuild == false){
            //说明已经delete过了
            return
        }

        node.isBuild = false;
        //this.nodeCount --
        this.nodeCountDelta --
    }

    deleteAllParents(){
        if(this.patentDelete){
            return
        }
        this.patentDelete = true

        if(this.head == null){
            console.log("this.head = null")
            return
        }
        let node = this.head

        while(node != null){
            let parent = node.parent
            if(parent == null){
                console.log("patent == null")
                node = node.nextNode
                continue
            }

            //不断向上删除parent，直到parent为空或者已经删除
            while(parent!=null){
                if(!parent.isBuild){
                    break
                }
                this.deleteNode(parent)
                parent = parent.patent
            }

            node = node.nextNode
        }
    }


    // 获取所有节点
    getNodes() {
        return this.nodes;
    }
}


class M4 {
    constructor(){
        this.max = -Infinity
        this.min = Infinity
        this.start_time = 0
        this.end_time = 0
        this.st_v = 0
        this.et_v = 0

        //一个M4代表一个像素列
        this.innerNodes = []    //像素列内的node
        this.stNodes = []   //像素列左边界的node index
        this.etNodes = []   //像素列内右边界node index

        this.stNodeIndex = null   //像素列左边界的node index
        this.etNodeIndex = null   //像素列内右边界node index


        //跟计算有关的
        this.alternativeNodesMax = null
        this.alternativeNodesMin = null;
        this.currentComputingNodeMax = null
        this.currentComputingNodeMin = null
        this.currentComputingIntervalMax = null
        this.currentComputingIntervalMin = null
        this.isCompletedMax = false;
        this.isCompletedMin = false;

        //跟计算均值有关
        this.stInterval = null
        this.etInterval = null
        this.minDLL = null
        this.maxDLL = null
        this.stNodes = []
        this.etNodes = []


        //error bound 相关
        this.errorPixels = 0

    }
    
}

class SCREEN_M4 {

    constructor(experiment,datasetname,quantity,symbolName,width,height,errorBound, func){
        this.M4_arrays = [],
        this.min_values = [],
        this.max_values = []
        
        this.sx = ''

        this.M4_array = null
        this.screenEnd = 0
        this.screenStart=0
        this.height=height
        this.width=width
        this.experiment = experiment
        this.datasetname = datasetname
        this.symbolName = symbolName
        this.errorBound = errorBound
        this.quantity = quantity
        this.func = func
        this.nodeReductionRatio = 0
        this.SQLtime = 0
        this.totalTime=0

        this.buildNodeRate = 0
        this.segmentTrees = null
        this.dataReductionRatio = 0

        this.maxNodeNum = 0
        this.nodeCount = 0
        this.memLimit = 0
        this.interact_type = ''
        this.columns = []



        this.exactMax=-Infinity
        this.exactMin=Infinity
        this.avgtMax = -Infinity
        this.avgtMin = Infinity
        this.candidateMax=-Infinity
        this.candidateMin=Infinity


        this.preError = 0
        this.deltaError = 0

        this.count=0

        //一些与时间相关的变量
        // 查询区间的开始、结束，对应数据库中的t，0开始
        this.screenEnd = 0
        this.screenStart=0

        //数据集对应的开始时间和结束时间的时间戳，时间间隔（秒），如 2015-1-1 0:0:0，时间间隔60秒
        this.dataStartTime = -1
        this.dataEndTime = -1
        this.dataDelta = -1
        this.dataCont = -1

        //所有区间的第一个区间开始时间戳，最后一个区间结束的时间戳
        this.globalIntervalStartTime = 1420041600
        this.globalIntervalEndTime = 1704038399

        // 查询区间的开始、结束,在该数据集汇总对应的时间戳
        this.screenStartTimestamp = 1420041600
        this.screenEndTimestamp = 1704038399

        this.globalIntervalStart=-1
        this.globalIntervalEnd=-1

        this.delta = 60

        //区间的长度，对应包含多少个数据点个数，而不是时间。
        this.intervalLength = 60 

        //迭代次数
        this.iterations = 0
    }
}


function computeM4TimeSE(width,timeRange){

    const res = []
    for(i = 0;i<width;i ++){
        res.push(new M4())
    }

    let globalStart = timeRange[0]
    let globalEnd = timeRange[1]

    //timeRangeLength个点，分给width个桶
    const timeRangeLength = globalEnd - globalStart + 1

    // 平均每个桶，分的点数
    const everyNum = timeRangeLength/width

    // 第一个M4，以globalStart开始
    res[0].start_time = globalStart;
    //res[0].end_time = Math.ceil(everyNum) - 1


    for(i = 1;i<width;i ++){

        // 当前M4开始，是初始开始+平均每个桶分的点数，向上取整
        res[i].start_time=globalStart + Math.ceil( i * everyNum)

        // 上一个M4结尾，是下一个M4开始-1
        res[i-1].end_time = res[i].start_time - 1

    }

    //最后一个M4，以globalEnd结尾
    res[width-1].end_time=globalEnd

    return res
}








//===============================




function getExtremeInInterval(min, max) {
    let min_pre_k = Math.ceil(min / Math.PI - 0.5);
    let max_pre_k = Math.floor(max / Math.PI - 0.5);

    let extreme = []
    for (let k = min_pre_k; k <= max_pre_k; k++) {
        extreme.push((k+0.5)*Math.PI);
    }

    return extreme;
}


class FunInfo{
    constructor(funName, extremes, params){
        this.funName = funName;
        this.params = params

        this.intervalRange = 0
        this.extremes = []
        this.mode = 'multi'
        if(extremes != null){
            this.extremes.push(...extremes)
        }


        //字典，key为函数名，如func1、func2_0；value为对应的极值点数组。对x函数，取[0],对x和y函数，分别[0]和[1]
        this.maxExtremes = {}
        this.minExtremes = {}

        this.init()
    };

    init(){

        this.maxExtremes = {}
        this.minExtremes = {}

        //初始化func1、func2_0等的极值点。
        this.maxExtremes['func1'] = []
        this.minExtremes['func1'] = []
        this.maxExtremes['func1'].push(-31.62)
        this.minExtremes['func1'].push(31.62)

        this.maxExtremes['boxcox_0'] = []
        this.minExtremes['boxcox_0'] = []

        this.maxExtremes['boxcox_1_2'] = []
        this.minExtremes['boxcox_1_2'] = []

        this.maxExtremes['boxcox_1'] = []
        this.minExtremes['boxcox_1'] = []

        this.maxExtremes['boxcox_2'] = []
        this.minExtremes['boxcox_2'] = []
        this.minExtremes['boxcox_2'].push(0)

        this.maxExtremes['func3'] = []
        this.minExtremes['func3'] = []

        this.maxExtremes['func4'] = []
        this.minExtremes['func4'] = []
        //todo  初始化其他函数的极值点。
        this.weights = weights
        
    }


    weightsum(nodes, destination, isLeaf) {
        //函数：sum(wi * xi)
        let weights = this.weights //[-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10]
        let sum = 0
        if(destination === 'min'){
            for(let i=0;i<nodes.length;i++){
                let node = nodes[i]
                let weight = weights[i]
                
                if(weight >0){
                    sum += weight*node.min
                }else{
                    sum += weight*node.max
                }
            }
        }else{
            for(let i=0;i<nodes.length;i++){
                let node = nodes[i]
                let weight = weights[i]

                if(weight <0){
                    sum += weight*node.min
                }else{
                    sum += weight*node.max
                }
            }
        }

        return sum
    }

    

    L2LN(nodes, destination, isLeaf) {
        //函数：sqrt[ sum(xi ^ 2) ]

        let sum = 0
        for (let node of nodes)
        {
            if (destination == "min")
            {
                let x = Math.log(node.min+1);
                let y = x * x;
                sum += y;
            }
            else
            {
                let x = Math.log(node.max+1);
                let y = x * x;
                sum += y;
            }
        }
        return Math.sqrt(sum)
    }

    L2(nodes, destination, isLeaf) {
        //函数：sqrt[ sum(xi ^ 2) ]

        let sum = 0
        if(destination === 'min'){
            for(let i=0;i<nodes.length;i++){
                let node = nodes[i]
                if(node.min * node.max < 0){
                    sum += 0
                }else{
                    sum += Math.min(node.min*node.min, node.max*node.max)
                }
            }
        }else{
            for(let i=0;i<nodes.length;i++){
                let node = nodes[i]
                sum += Math.max(node.min*node.min, node.max*node.max)
            }
        }
        return Math.sqrt(sum)
    }

    getmax(nodes, destination, isLeaf) {
        let maxofmax = -Infinity
        let maxofmin = -Infinity

        for(let i=0;i<nodes.length;i++){
            let node = nodes[i]
            maxofmax = Math.max(maxofmax, node.max)
            maxofmin = Math.max(maxofmin, node.min)
        }
        if(destination === 'min'){
            return maxofmin
        }else{
            return maxofmax
        }
    }

    func4(nodes, destination, isLeaf) {
        // 函数: (1 - sin x)(ln(|y|+1))^2
    
        if(isLeaf){
            let x = nodes[0].min
            let y=nodes[1].min
            return (1 - Math.sin(x)) * (Math.log(Math.abs(y)+1)) **2
        }
    
        const xRange = nodes[0];
        const yRange = nodes[1];
        //console.log(`xRange: ${Math.sin(xRange.min)}, ${Math.sin(xRange.max)}, yRange: ${yRange.min}, ${yRange.max}`)
    
        let x_min=Infinity, x_max=-Infinity, y_min, y_max = 0;
        if (xRange.max - xRange.min >= (2 * Math.PI)) {
            x_min = -1;
            x_max = 1;
        } else {
    
            let extreme = getExtremeInInterval(xRange.min, xRange.max);
            extreme.push(xRange.min)
            extreme.push(xRange.max)
            for (let i = 0; i < extreme.length; i++) {
                let tmp = Math.sin(extreme[i]);
            
                if(x_min>tmp){
                    x_min=tmp
                }
                if(x_max<tmp){
                    x_max=tmp
                }
            }
    
        }
    
    
        y_max = Math.max(Math.abs(yRange.min), Math.abs(yRange.max));
        if (yRange.min * yRange.max < 0) {
            y_min = 0;
        } else {
            y_min = Math.min(Math.abs(yRange.min), Math.abs(yRange.max));
        }
       //console.log(`x_min = ${x_min}\nx_max = ${x_max}\ny_min = ${y_min}\ny_max = ${y_max}\n`)
    
        if (destination === 'min') {
            return (1 - x_max) * (Math.log(y_min + 1)) ** 2;
        } else {
            return (1 - x_min) * (Math.log(y_max + 1)) ** 2;
        }
    }

    //所有的nodes全是数组，只是对x函数，取nodes[0];对x和y函数，分别nodes[0]和nodes[1]
    compute(funName, nodes, destination){
 
        if(destination == 'min'){
            let Xs = this.generateXs(nodes[0].min, this.minExtremes[funName], nodes[0].max)
            let min = Infinity

            for(let i=0;i<Xs.length;i++){
                let x = Xs[i]
                let y= this.getFunc(funName, x)

                if(min > y){
                    min = y
                }
            }

            return min
        }else{
            let Xs = this.generateXs(nodes[0].min, this.maxExtremes[funName], nodes[0].max)
            let max = -Infinity

            for(let i=0;i<Xs.length;i++){
                let x = Xs[i]
                let y= this.getFunc(funName, x)

                if(max < y){
                    max = y
                }
            }

            return max
        }
    }

    getFunc(funName, x){
        if(funName == 'func1'){
            return this.func1(x)
        }else if(funName == 'boxcox_0'){
            return this.boxcox_0(x)
        }else if(funName == 'boxcox_1_2'){
            return this.boxcox_1_2(x)
        }else if(funName == 'boxcox_1'){
            return this.boxcox_1(x)
        }else if(funName == 'boxcox_2'){
            return this.boxcox_2(x)
        }


        return this.compute_defult(x) 
    }

    func1(x){
        //0.001*x^3 - 3*x
        let y = 0.001*(x**3) - 3*x

        return y
    }

    boxcox_0(x){
        //Box-Cox,lambda = 0
        //logx
        if(x<=0){
            return 0
        }

        return Math.log(x)
    }
    
    boxcox_1_2(x){
        //Box-Cox,lambda =1/2
        //(x**(1/2) - 1)/(1/2)
        if(x<=0){
            x=0
        }

        return (x**(1/2) - 1)/(1/2)
    }
   
    boxcox_1(x){
        //Box-Cox,lambda = 1
        //x-1

        return x-1
    }
    
    boxcox_2(x){
        //Box-Cox,lambda = 2
        //(x**2 - 1)/2

        return (x**2 - 1)/2
    }

    
   


    // func1(nodes, destination){
    //     //0.001*x^3 - 3*x

    //     if(destination == 'min'){
    //         let Xs = this.generateXs(nodes[0].min, this.minExtremes['func1'], nodes[0].max)
    //         let min = Infinity

    //         for(let i=0;i<Xs.length;i++){
    //             let x = Xs[i]
    //             let y= 0.001*(x**3) - 3*x

    //             if(min > y){
    //                 min = y
    //             }
    //         }

    //         return min
    //     }else{
    //         let Xs = this.generateXs(nodes[0].min, this.maxExtremes['func1'], nodes[0].max)
    //         let max = -Infinity

    //         for(let i=0;i<Xs.length;i++){
    //             let x = Xs[i]
    //             let y= 0.001*(x**3) - 3*x

    //             if(max < y){
    //                 max = y
    //             }
    //         }

    //         return max
    //     }
    // }

    // func2_0(nodes, destination){
    //     //Box-Cox,lambda = 0
    // }

    // func2_1_2(nodes, destination){
    //     //Box-Cox,lambda =  1/2
    // }

    // func2_1(nodes, destination){
    //     //Box-Cox,lambda = 1
    // }

    // func2_2(nodes, destination){
    //     //Box-Cox,lambda = 2
    // }


    // func3(nodes, destination){
    //     //x^(1/(|y|+1))
    // }

    // func4(nodes, destination){
    //     //(1-sin x)(ln(|y|+1))^2
    // }

    func_variance(nodes, destination){
        //方差，若干个node，数量不固定，计算方差
    }



    //extremes是一个数组，返回一个数组，数组包含：min、max、以及extremes中在min和max之间的
    generateXs(min, extremes, max){

        let x = []

        if(extremes != null){
            for(let i=0;i<extremes.length;i++){
                if(extremes[i] > min && extremes[i] < max){
                    x.push(extremes[i])
                }
            }
        }

        x.push(min)
        x.push(max)

        return x
    }

    compute_defult(x){
        let y = x

        return y
    }
    //根据funName函数体，依次计算Xs的函数值，返回Ys数组
    computes_defult(Xs){

        let Ys=[]
        for(let i=0;i<Xs.length;i++)
        {
            //todo
            //Ys.push(sin()+cos()+....)
            let x=Xs[i]
            let y=this.compute_defult(x)

            Ys.push(y)
        }
        
        return Ys
    }

}


function outputM4(screen_m4){
    let M4_array=screen_m4.M4_array  

    const memoryUsage = process.memoryUsage();
    console.log(`RSS: ${memoryUsage.rss / 1024 / 1024} MB`);
    let rss = `memeoryuse: ${memoryUsage.rss / 1024 }`;

    for(let key in stats.callCounts){
        timetotal(key)
    }

    let SQLtime = getTotalTime('SQL.query.time')
    let totalTime = getTotalTime('totaltime')

    if(isNaN(SQLtime)){
        SQLtime = 0
    }

    console.log('experiment:',screen_m4.experiment
            ,'; table:',screen_m4.datasetname
            ,'; interacttype:',screen_m4.interact_type
            ,'; columns:',`${screen_m4.columns}`
            //,'; memLimit:',screen_m4.memLimit
            // ,', maxNodeNum:',screen_m4.segmentTrees[0].maxNodeNum
            ,'; nodeCount:',screen_m4.segmentTrees[0].nodeCount
            ,'; errorBound:',screen_m4.errorBound
            ,'; symbolName:',`${screen_m4.symbolName}`
            ,'; width:',screen_m4.width
            ,'; startTime:',screen_m4.screenStart
            ,'; endTime:',screen_m4.screenEnd
            ,'; iterations:',screen_m4.iterations
            ,'; totaltime:', totalTime.toFixed(3)+''
            ,'; sqltime:', SQLtime.toFixed(3)+''
            ,`; ${rss}`)
    //console.log(screen_m4.M4_arrays)
    if(screen_m4.M4_array != null){
        let writestr = `m4 info; globalMin: ${screen_m4.min_values} ;globalMax: ${screen_m4.max_values} ; experiment: ${screen_m4.experiment} ;interacttype: ${screen_m4.interact_type} ;table: ${screen_m4.datasetname} ;columns: ${screen_m4.columns} ;symbol: ${screen_m4.symbolName} ;width: ${screen_m4.width} ;height: ${screen_m4.height} ;startTime: ${screen_m4.screenStart} ; endTime: ${screen_m4.screenEnd} ;errorBound: ${screen_m4.errorBound} ; error: ${screen_m4.preError.toFixed(4)} ; memLimit: ${screen_m4.memLimit} ; totaltime: ${totalTime.toFixed(3)} ; sqltime: ${SQLtime.toFixed(3)} ; iterations: ${screen_m4.iterations} ; nodeCount: ${screen_m4.segmentTrees[0].nodeCount} ; ${rss}`

        for(let i=0;i<M4_array.length;i++){
            let m4 = M4_array[i]
            let m4str=`m4: ${i} sT: ${m4.start_time} ,eT: ${m4.end_time} ,sV: ${m4.st_v.toFixed(3)} ,eV: ${m4.et_v.toFixed(3)} ,min: ${m4.min.toFixed(3)} ,max: ${m4.max.toFixed(3)}`
    
            writestr = `${writestr}\n${m4str}`
    
        }
    
        //console.log(writestr)
        //${e}_${w}_${eb}_${str}_${ss}.m4
        fs.writeFileSync(`../m4_result/${screen_m4.experiment}_${screen_m4.interact_type}_${screen_m4.columns}_${screen_m4.width}_${screen_m4.memLimit}_${screen_m4.datasetname}_${screen_m4.screenStart}_${screen_m4.screenEnd}_${screen_m4.errorBound}_${screen_m4.symbolName}.m4`, writestr);
    }else{
        for(let m=0 ; m<screen_m4.M4_arrays.length;m++){
            M4_array = screen_m4.M4_arrays[m]

            let writestr = `m4 info; globalMin: ${screen_m4.min_values} ;globalMax: ${screen_m4.max_values} ; experiment: ${screen_m4.experiment} ,interacttype: ${screen_m4.interact_type} ,table: ${screen_m4.datasetname} ;columns: ${screen_m4.columns[m]} ;symbol: ${screen_m4.symbolName}#${screen_m4.func.params} ;width: ${screen_m4.width} ;height: ${screen_m4.height} ;startTime: ${screen_m4.screenStart} ; endTime: ${screen_m4.screenEnd} ;errorBound: ${screen_m4.errorBound} ; error: ${screen_m4.preError.toFixed(4)} ; memLimit: ${screen_m4.memLimit}`
    
            for(let i=0;i<M4_array.length;i++){
                let m4 = M4_array[i]
                let m4str=`m4: ${i} sT: ${m4.start_time} ,eT: ${m4.end_time} ,sV: ${m4.st_v.toFixed(3)} ,eV: ${m4.et_v.toFixed(3)} ,min: ${m4.min.toFixed(3)} ,max: ${m4.max.toFixed(3)}`
    
                writestr = `${writestr}\n${m4str}`
    
            }
    
            //console.log(writestr)
            //${e}_${w}_${eb}_${str}_${ss}.m4
            fs.writeFileSync(`../m4_result/${screen_m4.experiment}_${screen_m4.interact_type}_${screen_m4.columns[m]}_${screen_m4.width}_${screen_m4.memLimit}_${screen_m4.datasetname}_${screen_m4.screenStart}_${screen_m4.screenEnd}_${screen_m4.errorBound}_${screen_m4.symbolName}#${screen_m4.func.params}.m4`, writestr);
        }
    }

    


    // if(screen_m4.sx in ablationStudyDict){
    //     //console.log(ablationStudyDict)
    //     ablationStudyDict[screen_m4.sx]["buildNodeNum"] += segmentTrees[0].nodeCountDelta
    //     ablationStudyDict[screen_m4.sx]["totaltime"] += totalTime
    //     ablationStudyDict[screen_m4.sx]["sqltime"] += SQLtime
    // }else{
    //     ablationStudyDict[screen_m4.sx] = {}
    //     ablationStudyDict[screen_m4.sx]["buildNodeNum"] = segmentTrees[0].nodeCountDelta
    //     ablationStudyDict[screen_m4.sx]["totaltime"] = totalTime
    //     ablationStudyDict[screen_m4.sx]["sqltime"] = SQLtime
    // }

    // let ablationStudyResult = `ablationStudyResult: type:${screen_m4.sx}, buildNodeNum: ${segmentTrees[0].nodeCountDelta}, totaltime: ${totalTime.toFixed(3)} ,sqltime: ${SQLtime.toFixed(3)}, dataset:${screen_m4.datasetname}, function: ${screen_m4.symbolName}`
    // console.log(ablationStudyResult)


}

let columnsNum = 2
function mergeTables(table_name, columns, funInfo, tableType){
    let order = []
    order.push(`v${columns[0]}`)
    for(let i=1;i<columns.length;i++){
        if(columns[i] == '' || columns[i] == null){
            continue
        }

        if(funInfo.funName == 'func1' || funInfo.funName == 'boxcox_0' 
            || funInfo.funName == 'boxcox_1_2' || funInfo.funName == 'boxcox_1' || funInfo.funName == 'boxcox_2'
        ){
             //一元计算，table_name_others不取
             if(order.length >0){
                //二元计算，table_name_others只取1个
                break
            }

        }else if(funInfo.funName == '+' || funInfo.funName == '-' || funInfo.funName == '*' || funInfo.funName == '/'
             || funInfo.funName == 'func3' || funInfo.funName == 'func4' 
            || funInfo.funName == 'L22'){
            if(order.length >1){
                //二元计算，table_name_others只取1个
                break
            }
        }

        order.push(`v${columns[i]}`)
    }
    // if(table_name_others.length > 0){
    //     tables.push(...table_name_others)
    // }


    if(tableType == 'om3'){
 
        if(!table_name.endsWith('_om3')){
            table_name =  table_name + '_om3';
        }

    }else if(tableType == 'tv'){
        if(table_name.endsWith('_om3')){
            table_name = removeEndChar(table_name, '_om3');
        }
    }

    columns.splice(0, columns.length, ...order);

    columnsNum = columns.length
    //console.log('columns.length:',columns.length)

    return table_name

}


async function profile(func, ...params) {

    let start = performance.now();

    //console.time(func.name); // 开始计时
    const result = await func(...params); // 执行函数
    //console.timeEnd(func.name); // 结束计时并打印结果

let end = performance.now();

console.log('start:', ((end - start) / 1000).toFixed(3)+'s')

    return result;
}
 





const stats = {
    functionTimes: {},
    startTimes: {},
    callCounts: {}  // 新增用于记录调用次数
};

async function Experiments(experiment, startTime, endTime, table_name,dataCount, columns, symbol, width, height, mode, parallel, errorBound, interact_type, screen_m4) {

    //console.log('Experiments:',experiment, startTime, endTime, table_name, columns, symbol, width, height, mode, parallel, errorBound, interact_type, screen_m4)


    let datasetname = table_name
    datasetname = datasetname.replace(/_/g, '');  // 使用正则表达式去掉所有的下划线



    let symbolName = getSymbolName(symbol);

    //screen_m4 = new SCREEN_M4(experiment, datasetname, 0, symbolName, width, height, errorBound, null)
    screen_m4.estimateType = 'estimateType'
    screen_m4.sx = 'sx'
    screen_m4.screenStart = startTime
    screen_m4.screenEnd = endTime
    screen_m4.columns = [...columns]

    //eg:  symbol = '-;ave#week'   ' ;ave#week'  '+'
    let s = symbol.split(';')
    if(s.length > 1){
        symbol = s[0];
        let ag = s[1].split('#')
        let func = new FunInfo(ag[0],null,ag[1])
        screen_m4.func = func
    }else{
        screen_m4.func = null
    }


    errorBoundSatisfyCount = 0
    if (!isTreeCache) {
        treeCache = {}
    }


    genDatainfo(screen_m4)

    timeclear()
    timestart('totaltime')

    switch (experiment) {
        
        case 'DUCK-F-M4-THREAD-1':
            await Case8(table_name, columns, symbol, '', width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4, 1); 
            break;
        case 'DUCK-F-M4-THREAD-16':
            await Case8(table_name, columns, symbol, '', width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4, 16); 
            break;

        case 'DUCK-F-AM4-THREAD-1':
            await Case9(table_name, columns, symbol, '', width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4, 1); 
            break;

        case 'DUCK-F-AM4-THREAD-16':
            await Case9(table_name, columns, symbol, '', width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4, 16); 
            break;
    }
    
    timeend('totaltime')
    outputM4(screen_m4)


    const memoryUsage = process.memoryUsage();
    console.log(`RSS: ${memoryUsage.rss / 1024 / 1024} MB`);
}


function timeclear() {
    stats.functionTimes = {}
    stats.startTimes = {}
    stats.callCounts = {}

}

// 开始计时函数
function timestart(functionName) {
    stats.startTimes[functionName] = performance.now();
}

// 结束计时函数
function timeend(functionName) {
    if (!stats.startTimes[functionName]) {
        console.error(`No start time recorded for ${functionName}`);
        return;
    }

    const startTime = stats.startTimes[functionName];
    const endTime = performance.now();
    const timeSpent = endTime - startTime;

    // 更新总时间
    if (!stats.functionTimes[functionName]) {
        stats.functionTimes[functionName] = 0;
    }
    stats.functionTimes[functionName] += timeSpent;

    // 更新调用次数
    if (!stats.callCounts[functionName]) {
        stats.callCounts[functionName] = 0;
    }
    stats.callCounts[functionName]++;

    // 清除开始时间
    delete stats.startTimes[functionName];


    return timeSpent
}

function getTotalTime(functionName){
    return (stats.functionTimes[functionName]) / 1000.0
}

function getTotalCount(functionName){
    return stats.callCounts[functionName] || 0
}

function getAvgTime(functionName){
    const totalTime = stats.functionTimes[functionName];
    const count = stats.callCounts[functionName] || 0;

    if (totalTime !== undefined) {
        return totalTime/1000.0/count
    } else {
        console.log(`No timing data for ${functionName}`);
        return 0;
    }
}

// 输出统计数据函数
function timetotal(functionName) {
    const totalTime = stats.functionTimes[functionName];
    const count = stats.callCounts[functionName] || 0;
    
    if (totalTime !== undefined) {
        console.log(`Total time for ${functionName}: ${totalTime.toFixed(2)} ms, called ${count} times`);
    } else {
        console.log(`No timing data for ${functionName}`);
    }
}









 //====================以下都是实验代码================
 




function removeEndChar(str, charToRemove) {
    const regex = new RegExp(charToRemove + '$'); // 创建正则表达式，匹配结尾的字符
    return str.replace(regex, ''); // 替换为空字符串
}


function getWithQ(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4){


    let column1 = columns[0];
    let column_others = columns.slice(1);


    let sql_query = ` Q AS (select t as t, `
 
    if (symbol == '+' | symbol == '-' | symbol == '*'){
        sql_query = sql_query.concat(`(${column1}`);
        column_others.forEach(order => {
            sql_query = sql_query.concat(` ${symbol} ${order}`)
        });
        sql_query = sql_query.concat(`) AS v
            from ${table_name} `);
    } else if (symbol == '/'){
        sql_query = sql_query.concat(`(COALESCE(${column1}`);
        column_others.forEach(order => {
            sql_query = sql_query.concat(` ${symbol} nullif(${order}, 0)`)
        });
        sql_query = sql_query.concat(`, 0))AS v
            from ${table_name} `);
    }else if (symbol == 'mean'){
        let weight = 1 / (column_others.length + 1);
        sql_query = sql_query.concat(`${weight} * ${column1}`);
        column_others.forEach(order => {
            sql_query = sql_query.concat(` + ${weight} * ${order}`)
        });
        sql_query = sql_query.concat(` AS v
            from ${table_name} `);
    } else if (symbol == 'variance'){
        let weight = 1 / (column_others.length + 1);
        let mean = `(${weight} * ${column1}`;
        column_others.forEach(order => {
            mean = mean.concat(` + ${weight} * ${order}`)
        });
        mean = mean.concat(`)`);
        sql_query = sql_query.concat(`${weight} * (${column1} - ${mean})^2`);
        column_others.forEach(order => {
            sql_query = sql_query.concat(` + ${weight} * (${order} - ${mean})^2`)
        });
        sql_query = sql_query.concat(` AS v
            from ${table_name} `);
    } else if (symbol == 'boxcox_0'){
        sql_query = sql_query.concat(`COALESCE(LN(nullif(${column1}, 0)), 0) as v from ${table_name}\n`);
    } else if (symbol == 'boxcox_1_2'){
        sql_query = sql_query.concat(`(sqrt(${column1})-1) * 2 as v from ${table_name}\n`);
    } else if (symbol == 'boxcox_1'){
        sql_query = sql_query.concat(`${column1}-1 as v from ${table_name}\n`);
    } else if (symbol == 'boxcox_2'){
        sql_query = sql_query.concat(`(power(${column1}, 2)-1) / 2 as v from ${table_name}\n`);
    } else if (symbol == 'func1'){
        sql_query = sql_query.concat(`0.001 * power(${column1}, 3) - 3 * ${column1} as v from ${table_name}\n`);
    } else if (symbol == 'func4'){
        sql_query = sql_query.concat(`(1-sin(${column1}))*power(LN(abs(${column_others[0]})+1), 2) as v from ${table_name} `);
    } else if (symbol == 'weightsum'){
       sql_query = sql_query.concat(`${weights[0]} * ${column1}`);
       column_others.forEach((order, idx) => {
           sql_query = sql_query.concat(` + (${weights[idx + 1]}) * (${order})`);
       });
       sql_query = sql_query.concat(` AS v from ${table_name} `);
    } else if (symbol == 'L2' || symbol == 'L22'){
       sql_query = sql_query.concat(`SQRT( power(${column1}, 2)`);
       column_others.forEach(order => {
           sql_query = sql_query.concat(` + power(${order}, 2)`)
       });
       sql_query = sql_query.concat(` ) AS v
           from ${table_name} `);
    } else if (symbol == 'L2LN'){
       sql_query = sql_query.concat(`SQRT( power(LN(${column1}+1), 2)`);
       column_others.forEach(order => {
           sql_query = sql_query.concat(` + power(LN(${order}+1), 2)`)
       });
       sql_query = sql_query.concat(` ) AS v
           from ${table_name} `);
    }else if (symbol == 'max'){
       sql_query = sql_query.concat(`greatest(${column1}`);
       column_others.forEach(order => {
           sql_query = sql_query.concat(`, ${order}`)
       });
       sql_query = sql_query.concat(`) AS v
           from ${table_name} `);
    } else if (symbol == 'min'){
       sql_query = sql_query.concat(`least(${column1}`);
       column_others.forEach(order => {
           sql_query = sql_query.concat(`, ${order}`)
       });
       sql_query = sql_query.concat(`) AS v
           from ${table_name} `);
    }

    sql_query = sql_query.concat(`where t between ${startTime} and ${endTime})\n`);

    return sql_query;
    
}

function genAM4Q(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4){



    let sql_getM4 = 
        `SELECT min(t) as st, arg_min(v, t) as sv, max(t) as et, arg_max(v, t) as ev,
                min(v) as min_v, max(v) as max_v, arg_min(t, v),  arg_max(t, v),
                floor(${width}*(t-${startTime})/(${endTime}-${startTime}+1)) AS k
        FROM Q 
        GROUP BY k,
        order by st;`;


    return sql_getM4

}

function genM4Q(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4){

    let sql_getM4 = `SELECT t,v, k FROM Q JOIN
    (SELECT floor(${width}*(t-${startTime})::bigint / (${endTime}-${startTime}+1)) AS k,
           min(v) AS v_min, max(v) AS v_max,
           min(t) AS t_min, max(t) AS t_max
     FROM Q GROUP BY k) AS QA
ON k = floor(${width}*(t-${startTime})::bigint / (${endTime}-${startTime}+1))
    AND (v=v_min or v=v_max or t=t_min or t=t_max)
        order by t asc `

    return sql_getM4

}


const { DuckDBInstance } = require('@duckdb/node-api');
const { performance } = require('perf_hooks'); // Use performance for more accurate timing in Node.js


async function getDickConn(DB_NAME){
    let csvFile = `../data/${DB_NAME}.csv`
    let dbfile = `../data/${DB_NAME}.db`

    instance = await DuckDBInstance.create(dbfile); //my_database.db
    let createsql =  `CREATE TABLE IF NOT EXISTS ${DB_NAME} AS
            SELECT * FROM read_csv_auto('${csvFile}');
        `
    let connection = await instance.connect();
    result = await connection.run(createsql);

    return  connection
}

async function getTotalRowCount(connection, TABLE_NAME) {
    console.log(`Fetching total row count for table: ${TABLE_NAME}...`);
    try {
        const result = await connection.runAndReadAll(`SELECT COUNT(*) as count FROM ${TABLE_NAME}`);
        const rows = result.getRows();
        const count = Number(rows[0][0]);
        console.log(`Total rows: ${count}`);
        // DuckDB count returns BigInt, convert to Number if safe, or handle as BigInt if extremely large
        return Number(count);
    } catch (e) {
        console.error(`Error fetching row count for ${TABLE_NAME}:`, e);
        throw e; // Re-throw to stop benchmark if count fails
    }
}


function genM4_array(M4_array,result){

    //console.log(result)
    // console.log(M4_array)

    let i=0,j=0
    while(i < M4_array.length && j<result.length){
        let m4 = M4_array[i]
        let node = new SegmentTreeNode();
        node.sTime = Number(result[j][0])
        node.eTime = Number(result[j][0])
        node.min = Number(result[j][1])
        node.max = Number(result[j][1])

        // m4.min = Math.min(m4.min, m4.st_v, m4.et_v)
        // m4.max = Math.max(m4.max, m4.st_v, m4.et_v)
//console.log(node)

        let type = isContain(node, m4);
        
        // console.log(type)
        // break;

        if(type == -1){
            j++
            continue;
        }

        //叶子Node与M4左边界重合，该节点的值（因为是叶子节点，所以min=max）赋给该M4的左边界st_v
        if(type == -2){
            m4.st_v = node.min
            j++
            continue;
        }

        //叶子Node在M4内部，放到该M4的inner中
        if (type == -3) {
            if(node.min < m4.min){
                m4.min = node.min
            }
            if(node.max > m4.max){
                m4.max = node.max
            }
            j++
            continue;
        }

        //叶子Node与M4右边界重合，该节点的值（因为是叶子节点，所以min=max）赋给该M4的右边界et_v
        if (type == -4) {
            //m4.etNodeIndex = node.index
            m4.et_v = node.min
            j++
            i++
            continue;
        }
        if(type == -5){
            i++
            continue;
        }

    }

    for(let m4 of M4_array){
        m4.min = Math.min(m4.min, m4.st_v, m4.et_v)
        m4.max = Math.max(m4.max, m4.st_v, m4.et_v)
    }

}

 // 在数据库(DUCK)：两表在数据相加后，对结果做M4（M4）
 async function Case8(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4, threads=1){
   
    

    symbol = symbol.split(';')
    if (symbol.length > 1) {
        params = symbol[1].split(',')
    } else {
        params = []
    }

    let funInfo = new FunInfo(symbol[0],params)
    symbol = funInfo.funName
    weights = funInfo.weights

    table_name = mergeTables(table_name,columns,funInfo, 'tv')
    console.log("DUCK-F", table_name)
    
    let connection = await getDickConn(table_name)

    console.log('Database connection established.');
    let sqlQuery = ` SELECT current_setting('threads') AS threads;`;
    const result = await connection.runAndReadAll(sqlQuery);
    const currentThreads = result.getRows()[0][0];
    console.log(`Current threads: ${currentThreads}`);

    //let threads = 1;
    sqlQuery = `PRAGMA threads = ${threads};`;
    await connection.runAndReadAll(sqlQuery);
    console.log(`Set threads to ${threads}.`);

    sqlQuery = `SELECT COUNT(*) AS column_count FROM pragma_table_info('${table_name}');`;
    const column_count_result = await connection.runAndReadAll(sqlQuery);
    const column_count = Number(column_count_result.getRows()[0][0])-1;
    console.log(`Number of Columns: ${column_count}.`);



    if(endTime < 0){
        let num = totalNum = await getTotalRowCount(connection, table_name);
        endTime = num-1
     }

     let  sql_query = getWithQ(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4);
 
     sql_query = sql_query.concat(`where t between ${startTime} and ${endTime})\n`);
 
     let sql_getM4 = genM4Q(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4)
     

    
     sql = sql_query.concat(sql_getM4);
 
     console.log(`sql: ${sql}`);


     timestart('SQL.query.time');
     let result1 = await connection.runAndReadAll(sql);
    result1 = result1.getRows();
     timeend('SQL.query.time');
 

 
    let M4_array = computeM4TimeSE(width, [startTime, endTime])
    let MIN = Infinity;
    let MAX = -Infinity;
 
    genM4_array(M4_array,result1)

    let segmentTrees = [new SegmentTree()]
    screen_m4.segmentTrees = segmentTrees
    screen_m4.M4_array = M4_array
    screen_m4.dataReductionRatio =0

    const memoryUsage = process.memoryUsage();
    console.log(`RSS: ${memoryUsage.rss / 1024 / 1024} MB`);

     return {
         M4_array: M4_array,
         min_value: MIN,
         max_value: MAX
     }

 

}

 // 在数据库(DUCK)：两表在数据相加后，对结果做M4（AM4）
 async function Case9(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4, threads=1){
   

    symbol = symbol.split(';')
    if (symbol.length > 1) {
        params = symbol[1].split(',')
    } else {
        params = []
    }

    let funInfo = new FunInfo(symbol[0],params)
    symbol = funInfo.funName
    weights = funInfo.weights

    table_name = mergeTables(table_name,columns,funInfo, 'tv')
    console.log("DUCK-F", table_name)
    
    let connection = await getDickConn(table_name)

    console.log('Database connection established.');
    let sqlQuery = ` SELECT current_setting('threads') AS threads;`;
    const result = await connection.runAndReadAll(sqlQuery);
    const currentThreads = result.getRows()[0][0];
    console.log(`Current threads: ${currentThreads}`);

    //let threads = 1;
    sqlQuery = `PRAGMA threads = ${threads};`;
    await connection.runAndReadAll(sqlQuery);
    console.log(`Set threads to ${threads}.`);

    sqlQuery = `SELECT COUNT(*) AS column_count FROM pragma_table_info('${table_name}');`;
    const column_count_result = await connection.runAndReadAll(sqlQuery);
    const column_count = Number(column_count_result.getRows()[0][0])-1;
    console.log(`Number of Columns: ${column_count}.`);


    // sqlQuery = `SELECT current_setting('memory_limit');`;
    // let r = await connection.runAndReadAll(sqlQuery);
    
    // console.log(`memory_limit: ${r.getRows()}.`);



    if(endTime < 0){
        let num = totalNum = await getTotalRowCount(connection, table_name);
        endTime = num-1
     }



     timestart('SQL.query.time');
     let  sql_query = getWithQ(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4);
 
     sql_query = `WITH ${sql_query} `

     console.log(`with Q:\n ${sql_query}`);
 
    //  let sql_getM4 = genM4Q(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4)
     

     let sql_getM4 = genAM4Q(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4)
     

    
     sql = sql_query.concat(sql_getM4);
 
     console.log(`sql: ${sql}`);


     let result1 = await connection.runAndReadAll(sql);
    result1 = result1.getRows();

 

     //console.log(result1)
 
    let M4_array = computeM4TimeSE(width, [startTime, endTime])
    let MIN = Infinity;
    let MAX = -Infinity;
 
    this.max = -Infinity
    this.min = Infinity
    this.start_time = 0
    this.end_time = 0
    this.st_v = 0
    this.et_v = 0


    for(let i = 0;i < M4_array.length;i++){
        let m4 = M4_array[i]
         m4.start_time = Number(result1[i][0])
         m4.st_v = Number(result1[i][1])
         m4.end_time = Number(result1[i][2])
         m4.et_v = Number(result1[i][3])
         m4.min = Number(result1[i][4])
         m4.max = Number(result1[i][5])
    }

    let segmentTrees = [new SegmentTree()]
    screen_m4.segmentTrees = segmentTrees
    screen_m4.M4_array = M4_array
    screen_m4.dataReductionRatio =0

    timeend('SQL.query.time');

    const memoryUsage = process.memoryUsage();
    console.log(`RSS: ${memoryUsage.rss / 1024 / 1024} MB`);

     return {
         M4_array: M4_array,
         min_value: MIN,
         max_value: MAX
     }
}



 // 在数据库(DUCK)：两表在数据相加后，对结果做M4（AM4）
 async function Case10(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4, threads=1){
   
    let result1
    let memoryUsage = process.memoryUsage();
    console.log(`start: RSS: ${memoryUsage.rss / 1024 / 1024} MB`);

    symbol = symbol.split(';')
    if (symbol.length > 1) {
        params = symbol[1].split(',')
    } else {
        params = []
    }

    let funInfo = new FunInfo(symbol[0],params)
    symbol = funInfo.funName
    weights = funInfo.weights

    table_name = mergeTables(table_name,columns,funInfo, 'tv')
    console.log("DUCK-F", table_name)
    
    let connection = await getDickConn(table_name)

    console.log('Database connection established.');
    let sqlQuery = ` SELECT current_setting('threads') AS threads;`;
    const result = await connection.runAndReadAll(sqlQuery);
    const currentThreads = result.getRows()[0][0];
    console.log(`Current threads: ${currentThreads}`);

    //let threads = 1;
    sqlQuery = `PRAGMA threads = ${threads};`;
    await connection.runAndReadAll(sqlQuery);
    console.log(`Set threads to ${threads}.`);

    sqlQuery = `SELECT COUNT(*) AS column_count FROM pragma_table_info('${table_name}');`;
    const column_count_result = await connection.runAndReadAll(sqlQuery);
    const column_count = Number(column_count_result.getRows()[0][0])-1;
    console.log(`Number of Columns: ${column_count}.`);


    // sqlQuery = `SELECT current_setting('memory_limit');`;
    // let r = await connection.runAndReadAll(sqlQuery);
    
    // console.log(`memory_limit: ${r.getRows()}.`);



    if(endTime < 0){
        let num = totalNum = await getTotalRowCount(connection, table_name);
        endTime = num-1
     }



     timestart('SQL.query.time');
     let  sql_query = getWithQ(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4);
 
     //sql_query = `WITH ${sql_query} `

     //console.log(`with Q:\n ${sql_query}`);
 
    //  let sql_getM4 = genM4Q(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4)
     

    sql_query = `CREATE TEMPORARY TABLE ${sql_query} `

    console.log(`CREATE TEMPORARY Q:\n ${sql_query}`);

    result1 = await connection.runAndReadAll(sql_query);

     let sql_getM4 = genAM4Q(table_name,columns,symbol,params,width,height,mode,parallel,errorBound,startTime,endTime, interact_type,screen_m4)
     
     memoryUsage = process.memoryUsage();
    console.log(`TEMPORARY: RSS: ${memoryUsage.rss / 1024 / 1024} MB`);

    
     //sql = sql_query.concat(sql_getM4);
 
     console.log(`sql: ${sql_getM4}`);


     result1 = await connection.runAndReadAll(sql_getM4);
    result1 = result1.getRows();

    memoryUsage = process.memoryUsage();
    console.log(`sql_getM4: RSS: ${memoryUsage.rss / 1024 / 1024} MB`);
 

     //console.log(result1)
 
    let M4_array = computeM4TimeSE(width, [startTime, endTime])
    let MIN = Infinity;
    let MAX = -Infinity;
 
    this.max = -Infinity
    this.min = Infinity
    this.start_time = 0
    this.end_time = 0
    this.st_v = 0
    this.et_v = 0


    for(let i = 0;i < M4_array.length;i++){
        let m4 = M4_array[i]
         m4.start_time = Number(result1[i][0])
         m4.st_v = Number(result1[i][1])
         m4.end_time = Number(result1[i][2])
         m4.et_v = Number(result1[i][3])
         m4.min = Number(result1[i][4])
         m4.max = Number(result1[i][5])
    }

    let segmentTrees = [new SegmentTree()]
    screen_m4.segmentTrees = segmentTrees
    screen_m4.M4_array = M4_array
    screen_m4.dataReductionRatio =0

    timeend('SQL.query.time');

    memoryUsage = process.memoryUsage();
    console.log(`finish RSS: ${memoryUsage.rss / 1024 / 1024} MB`);

     return {
         M4_array: M4_array,
         min_value: MIN,
         max_value: MAX
     }
}




async function getPlans(){
    const fs = require('fs');

    let plans = {}
    // 读取文件内容
    const content = fs.readFileSync('./plans.csv', 'utf8');
    
    // 将内容按行分割
    const lines = content.split('\n');
    
    // 按行处理
    lines.forEach(line => {
        //console.log(`处理行: ${line}`);
        // 在这里进行对每行的操作，例如解析或处理数据

        let items = line.split(',')
        if(items.length > 3){

            let table = items[0]
            if(table in plans){
                plans[table].push(items)
            }else{
                plans[table]=[]
                plans[table].push(items)
            }

        }

    });
    

    return plans
}


function genDatainfo(screen_m4){

    let func = screen_m4.func
    if(screen_m4.datasetname == 'nycdata'){

        // todo 后面根据数据选取的时间段进行调整。
        screen_m4.globalIntervalStartTime = 1420041600
        screen_m4.globalIntervalEndTime = 1704038399
        screen_m4.delta = 60

        if(func == null || screen_m4.func.funName==''){
            return
        }

        if(func.funName == 'ave' || func.funName == 'sum'){
            if(func.params == 'hour'){
                screen_m4.intervalLength = 60
            }else if(func.params == 'day'){
                screen_m4.intervalLength = 60 * 24
            }else if(func.params == 'week'){
                screen_m4.intervalLength = 60 * 24 * 7
            }else if(func.params == 'month'){
                screen_m4.intervalLength = 60 * 24 * 30
            }else if(func.params == 'year'){
                screen_m4.intervalLength = 60 * 24 * 365
            }
        }
    }
}



async function test(){
console.log("test")
}




async function static(){



    let estimates = ["d"]//["d", "b", "c", "e", "f"]
    let ablationStudy = ["s1", "s2", "s3", "s4"]

    let experiments = ["ours","PG-F","OM3","PG-M4", "IFX-M4", "IFX-F"]
    let datasets = ["synthetic_1m 1,2,3,4,5", "synthetic_2m 1,2,3,4,5", "synthetic_4m 1,2,3,4,5", "synthetic_8m 1,2,3,4,5", "synthetic_16m 1,2,3,4,5", "synthetic_32m 1,2,3,4,5", "synthetic_64m 1,2,3,4,5", "synthetic_128m 1,2,3,4,5", "synthetic_256m 1,2,3,4,5", "synthetic512m5v 1,2,3,4,5"
        , "nycsecond 7,9,1,2,3,4,5,6,8,10,11", "sensordata 5,4,3,2,1,7,6"]
    let functions=["func1", "boxcox_0", "+", "-", "*", "/", "mean", "variance", "max", "L2", "weightsum", "L22", "L2LN"]
    let widths = [1200]
    
    let errorbounds = [0.1, 0.05, 0.01, 0]

    let timelimits = [1]//[0.1, 0.5, 1]
    let Mlimit = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048]


    ablationStudy = ["s3"]
     // functions=["/"]

     memeryCache = 1024 * 1024*1024 // 1G
     timelimit = 1000


     isMergeTable = true
     isTreeCache = false
     isParallel = false


     // =================for test=======================
     //datasets = ["nycdata 7,9,1,2,3,4,5,6,8,10,11", "traffic 1,2,3,4,5,6,7,8,9,10"]
      //datasets = ["synthetic_1m 1,2,3,4,5"]
    //   datasets = ["synthetic_1m 1,2,3,4,5", "synthetic_2m 1,2,3,4,5", "synthetic_4m 1,2,3,4,5"
    //     , "nycdata 7,9,1,2,3,4,5,6,8,10,11", "soccerdata 5,3,1,2,4,6", "stockdata 9,5,1,2,3,4,6,7,8,10", "traffic 1,2,3,4,5,6,7,8,9,10"
    //     , "synthetic_8m 1,2,3,4,5", "synthetic_16m 1,2,3,4,5", "sensordata 5,4,3,2,1,7,6", "synthetic_32m 1,2,3,4,5", "synthetic_64m 1,2,3,4,5"]

    // datasets = ["synthetic_8m 1,2,3,4,5", "synthetic_16m 1,2,3,4,5"
    //     , "nycdata 7,9,1,2,3,4,5,6,8,10,11", "soccerdata 5,3,1,2,4,6", "stockdata 9,5,1,2,3,4,6,7,8,10", "traffic 1,2,3,4,5,6,7,8,9,10"
    //     ]

    //     datasets = ["nycdata 7,9,1,2,3,4,5,6,8,10,11"]
    //datasets = ["synthetic_1m 1,2,3,4,5", "synthetic_2m 1,2,3,4,5", "synthetic_4m 1,2,3,4,5", "synthetic_8m 1,2,3,4,5"]
    datasets = [
        "sensordata 5,4,3,2,1,7,6"
        , "nycdata 7,9,1,2,3,4,5,6,8,10,11"
        , "soccerdata 5,3,1,2,4,6"
         ,"stockdata 9,5,1,2,3,4,6,7,8,10"
        //  ,"traffic 1,2,3,4,5,6,7,8,9,10"
        // , "synthetic_1m 1,2,3,4,5"
        // , "synthetic_2m 1,2,3,4,5"
        // , "synthetic_4m 1,2,3,4,5"
        // ,"synthetic_8m 1,2,3,4,5"
        // ,"synthetic_16m 1,2,3,4,5"
        // ,"synthetic_32m 1,2,3,4,5"
        // ,"synthetic_64m 1,2,3,4,5"
        // ,"synthetic_128m 1,2,3,4,5"
        // ,"synthetic_256m 1,2,3,4,5"
        // // ,"synthetic512m3v 1,2,3"
        // // ,"synthetic1b3v 1,2,3"
        //  ,"synthetic512m5v 1,2,3,4,5"
        //  ,"synthetic1b5v 1,2,3,4,5"
    ]
    experiments = ["DUCK-F-AM4-THREAD-1", "DUCK-F-AM4-THREAD-16"
        , "DUCK-F-M4-THREAD-1", "DUCK-F-M4-THREAD-16"]

    experiments = ["DUCK-F-AM4-THREAD-16"]
    functions=["func1", "boxcox_0", "+", "-", "*", "/", "mean", "variance", "max", "L2", "weightsum", "L22", "L2LN"]
      widths = [200,400,600,800,1000,1200]
      errorbounds = [0]
    // =================for test=======================

//experiments = ["ours"]

// for(let tl of timelimits){
//     timelimit = tl*1000
// for(m of Mlimit){
//     memeryCache= m * 1024*1024 //m MB

for(let experiment of experiments){
    for(let errorBound of errorbounds){

        for(let data of datasets){
            data = data.split(' ')
            let table_name = data[0]
            let dataCount = 0;//await getCount(table_name)
            let repeat = 0

            console.log(table_name)
            for (let symbol of functions) {


                //for (let width of widths) {
                for(let i=0;i<widths.length;i++){
                    let width = widths[i]

                
                    let mode = 'single'
                    let height = 600
                    let startTime = 0
                    let endTime = -1
                    let interact_type = 'null'
                    let parallel = 1
                    //let errorBound = 0.05
                    let screen_m4
                    //let maxEndTime = await getCount(table_name) - 1


                    //console.log(experiment, table_name)


                    let datasetname = table_name
                    datasetname = datasetname.replace(/_/g, '');  // 使用正则表达式去掉所有的下划线

                    let columns = data[1].split(',')


                    let symbolName = getSymbolName(symbol);
                    //console.log(i, plan, startTime, endTime, symbol, width)

                    screen_m4 = new SCREEN_M4(experiment, datasetname, 0, symbolName, width, height, errorBound)

                    screen_m4.interact_type = 'static'
                    screen_m4.estimateType = 'estimateType'
                    screen_m4.sx = 'sx'
                    screen_m4.screenStart = startTime
                    screen_m4.screenEnd = endTime
                    screen_m4.columns = [...columns]



                    errorBoundSatisfyCount = 0
                    if (!isTreeCache) {
                        treeCache = {}
                    }


                
                    await Experiments(experiment, startTime, endTime, table_name,dataCount, columns, symbol, width, height, mode, parallel, errorBound, interact_type, screen_m4);
 
                }

            }

        }    

        if(experiment != 'ours'){
            break
        }
    }

}

    //console.log(ablationStudyDict)
 
}


async function linenum(){



    let estimates = ["d"]//["d", "b", "c", "e", "f"]
    let ablationStudy = ["s1", "s2", "s3", "s4"]

    let experiments = ["ours","PG-F","OM3","PG-M4","IFX-M4", "IFX-F"]
    let datasets = ["synthetic10k2v 1,2", "synthetic100k2v 1,2", "synthetic1m2v 1,2", "synthetic10m2v 1,2", "synthetic100m2v 1,2"]
    let functions=["+"]
    let widths = [600]
    
    let errorbounds = [0.05]

    let timelimits = [1]//[0.1, 0.5, 1]
    let Mlimit = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048]


    ablationStudy = ["s3"]
     // functions=["/"]

     memeryCache = 1024 * 1024*1024 // 1G
     timelimit = 1000


     isMergeTable = true
     isTreeCache = false
     isParallel = false


     // for test
     //datasets = ["nycdata 7,9,1,2,3,4,5,6,8,10,11", "traffic 1,2,3,4,5,6,7,8,9,10"]
      experiments = ["ours"]
      datasets = ["synthetic10k2v 1,2", "synthetic100k2v 1,2", "synthetic1m2v 1,2", "synthetic10m2v 1,2", "synthetic100m2v 1,2"]

      datasets = ["synthetic100m2v 1,2"]
    //  functions=["weightsum", "L2"]



//experiments = ["ours"]

// for(let tl of timelimits){
//     timelimit = tl*1000
// for(m of Mlimit){
//     memeryCache= m * 1024*1024 //m MB

for(let experiment of experiments){
    for(let errorBound of errorbounds){
        for(let data of datasets){
            data = data.split(' ')
            let table_name = data[0]
            let dataCount = 0;//await getCount(table_name)
            for (let symbol of functions) {
                let repeat = 0
                //for (let width of widths) {
                for(let i=0;i<widths.length;i++){
                    let width = widths[i]

                
                    let mode = 'single'
                    let height = 600
                    let startTime = 0
                    let endTime = -1
                    let interact_type = 'null'
                    let parallel = 1
                    //let errorBound = 0.05
                    let screen_m4
                    //let maxEndTime = await getCount(table_name) - 1


                    //console.log(experiment, table_name)


                    let datasetname = table_name
                    datasetname = datasetname.replace(/_/g, '');  // 使用正则表达式去掉所有的下划线


                    let columns = data[1].split(',')

                    let symbolName = getSymbolName(symbol);
                    //console.log(i, plan, startTime, endTime, symbol, width)

                    screen_m4 = new SCREEN_M4(experiment, datasetname, 0, symbolName, width, height, errorBound)
                    screen_m4.estimateType = 'estimateType'

                    screen_m4.interact_type = 'linenum'
                    screen_m4.sx = 'sx'
                    screen_m4.screenStart = startTime
                    screen_m4.screenEnd = endTime
                    screen_m4.columns = [...columns]



                    errorBoundSatisfyCount = 0
                    if (!isTreeCache) {
                        treeCache = {}
                    }


                
                    await Experiments(experiment, startTime, endTime, table_name,dataCount, columns, symbol, width, height, mode, parallel, errorBound, interact_type, screen_m4);

                    console.log('内存使用情况：', process.memoryUsage().rss/1024/1024, 'MB');
 
                }

            }

        }    

        if(experiment != 'ours'){
            break
        }
    }

}

    //console.log(ablationStudyDict)
 
}

async function tsnum(){



    let estimates = ["d"]//["d", "b", "c", "e", "f"]
    let ablationStudy = ["s1", "s2", "s3", "s4"]

    let experiments = ["DUCK-F-AM4-THREAD-24"]
    let datasets = [
         "synthetic128m16v 1"
        ,"synthetic128m16v 1,2"
        ,"synthetic128m16v 1,2,3"
        ,"synthetic128m16v 1,2,3,4"
        ,"synthetic128m16v 1,2,3,4,5"
        ,"synthetic128m16v 1,2,3,4,5,6"
        ,"synthetic128m16v 1,2,3,4,5,6,7"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8,9"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8,9,10"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8,9,10,11"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8,9,10,11,12"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8,9,10,11,12,13"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15"
        ,"synthetic128m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16"]
    // let datasets = [
    //     "synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12,13"
    //    ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14"
    //    ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15"
    //    ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16"]
    let functions=["L2LN"]
    let widths = [600]
    
    let errorbounds = [0.05]

    let timelimits = [1]//[0.1, 0.5, 1]
    let Mlimit = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048]


    ablationStudy = ["s3"]
     // functions=["/"]

     memeryCache = 1024 * 1024*1024 // 1G
     timelimit = 1000


     isMergeTable = true
     isTreeCache = false
     isParallel = false


     // for test
    //   datasets = [
    //     "synthetic10m16v 1"
    //     ,"synthetic10m16v 1,2"
    //     ,"synthetic10m16v 1,2,3"
    //     ,"synthetic10m16v 1,2,3,4"
    //     ,"synthetic10m16v 1,2,3,4,5"
    //     ,"synthetic10m16v 1,2,3,4,5,6"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8,9"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12,13"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15"
    //     ,"synthetic10m16v 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16"
    // ]
    // functions=["mean"]
    // experiments = ["ours"]
    // //  experiments = ["IFX-F"]
    // //  functions=["weightsum", "L2"]



//experiments = ["ours"]

// for(let tl of timelimits){
//     timelimit = tl*1000
// for(m of Mlimit){
//     memeryCache= m * 1024*1024 //m MB

for(let experiment of experiments){
    for(let errorBound of errorbounds){
        for(let data of datasets){
            data = data.split(' ')
            let table_name = data[0]
            let dataCount = 0;//await getCount(table_name)
            for (let symbol of functions) {
                let repeat = 0
                //for (let width of widths) {
                for(let i=0;i<widths.length;i++){
                    let width = widths[i]

                
                    let mode = 'single'
                    let height = 600
                    let startTime = 0
                    let endTime = -1
                    let interact_type = 'null'
                    let parallel = 1
                    //let errorBound = 0.05
                    let screen_m4
                    //let maxEndTime = await getCount(table_name) - 1


                    //console.log(experiment, table_name)


                    let datasetname = table_name
                    datasetname = datasetname.replace(/_/g, '');  // 使用正则表达式去掉所有的下划线



                    let columns = data[1].split(',')
                    let symbolName = getSymbolName(symbol);
                    //console.log(i, plan, startTime, endTime, symbol, width)

                    screen_m4 = new SCREEN_M4(experiment, datasetname, 0, symbolName, width, height, errorBound)

                    screen_m4.interact_type = 'tsnum'
                    screen_m4.estimateType = 'estimateType'
                    screen_m4.sx = 'sx'
                    screen_m4.screenStart = startTime
                    screen_m4.screenEnd = endTime
                    screen_m4.columns = [...columns]



                    errorBoundSatisfyCount = 0
                    if (!isTreeCache) {
                        treeCache = {}
                    }


                
                    await Experiments(experiment, startTime, endTime, table_name,dataCount, columns, symbol, width, height, mode, parallel, errorBound, interact_type, screen_m4);
 
                }

            }

        }    

        if(experiment != 'ours'){
            break
        }
    }

}

    //console.log(ablationStudyDict)
 
}



async function interactions(){



    let estimates = ["d"]//["d", "b", "c", "e", "f"]
    let ablationStudy = ["s1", "s2", "s3", "s4"]

    let experiments = ["ours","case2","case3","case4","case5", "case6"]
    let datasets = ["nycdata 7,9,1,2,3,4,5,6,8,10,11", "synthetic_8m 1,2,3,4,5", "sensordata 5,4,3,2,1,7,6", "soccerdata 5,3,1,2,4,6", "stockdata 9,5,1,2,3,4,6,7,8,10", "traffic 1,2,3,4,5,6,7,8,9,10", "synthetic_1m 1,2,3,4,5", "synthetic_2m 1,2,3,4,5", "synthetic_4m 1,2,3,4,5", "synthetic_16m 1,2,3,4,5", "synthetic_32m 1,2,3,4,5", "synthetic_64m 1,2,3,4,5", "synthetic_128m 1,2,3,4,5", "synthetic_256m 1,2,3,4,5"]
    let functions=["func1", "boxcox_0", "+", "-", "*", "/", "func4", "mean", "variance"]
    let widths = [200, 400, 600, 800, 1000, 1200]
    let errorbounds = [0.1, 0.05, 0.01, 0]
    
    let timelimits = [1]
    let Mlimit = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048]



    //datasets = ["synthetic_256m 1,2,3,4,5"]
    ablationStudy = ["s3"]
     // functions=["/"]

     memeryCache = 10240 * 1024*1024 // 10G
     timelimit = 1000


     isMergeTable = true
     isTreeCache = true
     
     isParallel = false
     isMemLimit = false

     datasets = ["nycdata 7,9,1,2,3,4,5,6,8,10,11"
        ,"nycsecond 7,9,1,2,3,4,5,6,8,10,11"
        ,"soccerdata 5,3,1,2,4,6"
        ,"sensordata 5,4,3,2,1,7,6"
        ,"stockdata 9,5,1,2,3,4,6,7,8,10"
        ,"traffic 1,2,3,4,5,6,7,8,9,10"
       , "synthetic_1m 1,2,3,4,5"
       , "synthetic_2m 1,2,3,4,5"
       , "synthetic_4m 1,2,3,4,5"
       ,"synthetic_8m 1,2,3,4,5"
       ,"synthetic_16m 1,2,3,4,5"
       ,"synthetic_32m 1,2,3,4,5"
       ,"synthetic_64m 1,2,3,4,5"
       ,"synthetic_128m 1,2,3,4,5"
       ,"synthetic_256m 1,2,3,4,5"
       ,"synthetic512m3v 1,2,3"
       ,"synthetic1b3v 1,2,3"
        ,"synthetic512m5v 1,2,3,4,5"
        ,"synthetic1b5v 1,2,3,4,5"
   ]
    experiments = ["DUCK-F-AM4-THREAD-1", "DUCK-F-AM4-THREAD-16"
       , "DUCK-F-M4-THREAD-1", "DUCK-F-M4-THREAD-16"]
    experiments = ["DUCK-F-AM4-THREAD-16"]
//functions=["mean", "variance"]
Mlimit = [128, 256, 512, 1024, 2048]
// for(let tl of timelimits){
//     timelimit = tl*1000
//for(let m of Mlimit){
//    memeryCache= m * 1024*1024 //m MB
//for(let errorBound of errorbounds){

let allplans = await getPlans()
//console.log(plans)
//return 

    for (let data of datasets) {
        data = data.split(' ')
        let table_name = data[0]
        let dataCount = 0;//await getCount(table_name)

        let plans = allplans[table_name]
        if(plans == null){
            continue
        }
        //console.log(table_name,plans.length);continue

        for (let experiment of experiments) {

            for(let plan of plans){
                console.log(plan)
                let interact_type = plan[1]
                let columns = plan[2].split('I').map(Number)

                let startTime = parseInt(plan[3])
                let endTime = parseInt(plan[4])
                let symbol = plan[5]
                let symbolName = symbol
                let width = parseInt(plan[6])


                let mode = 'single'
                let height = 600
                //let interact_type = 'null'
                let parallel = 1
                let errorBound = 0.05


        


                let datasetname = table_name
                datasetname = datasetname.replace(/_/g, '');  // 使用正则表达式去掉所有的下划线

                let params =null
                let s = symbol.split('#')
                if (s.length > 1) {
                    params = s[1]
                } else {
                    params = []
                }
            
                let funInfo = new FunInfo(s[0],null,params)
                symbol = funInfo.funName


                symbol = getFuncName(symbolName);
                symbolName = getSymbolName(symbol);
                //console.log(i, plan, startTime, endTime, symbol, width)
                
                let screen_m4 = new SCREEN_M4(experiment, datasetname, 0, symbolName, width, height, errorBound, funInfo)
                screen_m4.estimateType = 'estimateType'
                screen_m4.sx = 'sx'
                screen_m4.screenStart = startTime
                screen_m4.screenEnd = endTime
                screen_m4.interact_type = "interaction-" + interact_type
                //screen_m4.memLimit = `${m}MB`
                screen_m4.columns = `${columns.join('I')}`
                genDatainfo(screen_m4)
                

                errorBoundSatisfyCount = 0
                if (!isTreeCache) {
                    treeCache = {}
                }


                // timeclear()
                // timestart('totaltime')
                await Experiments(experiment, startTime, endTime, table_name, dataCount, columns, symbol, width, height, mode, parallel, errorBound, interact_type, screen_m4);
                // timeend('totaltime');

                // outputM4(screen_m4)

            }
        }

            
    }
   





    //console.log(ablationStudyDict)

}





function getFuncName(symbol) {
    let symbolName = symbol.split(';')[0];

    if (symbolName == 'plus') {
        symbolName = '+';
    } else if (symbolName == 'minus') {
        symbolName = '-';
    } else if (symbolName == 'multi') {
        symbolName = '*';
    } else if (symbolName == 'devide') {
        symbolName = '/';
    } else if (symbolName == 'boxcox0') {
        symbolName = 'boxcox_0';
    } else if (symbolName == 'boxcox12') {
        symbolName = 'boxcox_1_2';
    } else if (symbolName == 'boxcox1') {
        symbolName = 'boxcox_1';
    } else if (symbolName == 'boxcox2') {
        symbolName = 'boxcox_2';
    }
    return symbolName;
}





function getSymbolName(symbol) {
    let ss = symbol.split(';')
    let symbolName = ss[0];

    if (symbolName == '+') {
        symbolName = 'plus';
    } else if (symbolName == '-') {
        symbolName = 'minus';
    } else if (symbolName == '*') {
        symbolName = 'multi';
    } else if (symbolName == '/') {
        symbolName = 'devide';
    } else if (symbolName == 'boxcox_0') {
        symbolName = 'boxcox0';
    } else if (symbolName == 'boxcox_1_2') {
        symbolName = 'boxcox12';
    } else if (symbolName == 'boxcox_1') {
        symbolName = 'boxcox1';
    } else if (symbolName == 'boxcox_2') {
        symbolName = 'boxcox2';
    }


    if(ss.length > 1){
        symbolName = `${symbolName}#${ss[1]}`
    }
    return symbolName;
}

function generateTwoUniqueRandomNumbers(min, max) {
    let num1 = Math.floor(Math.random() * (max - min + 1)) + min;
    let num2;

    // 确保 num2 不等于 num1
    do {
        num2 = Math.floor(Math.random() * (max - min + 1)) + min;
    } while (num2 === num1);

    return [num1, num2];
}


function arraysAreEqual(arr1, arr2) {
    // 判断长度是否相同
    if (arr1.length != arr2.length) {
        return false;
    }

    // 遍历数组中的每一项，逐一比较
    for (let i = 0; i < arr1.length; i++) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }

    // 如果所有元素都相同，则返回 true
    return true;
}



let weights = [
    0.58, 0.04, 0.94,
    0.57, 0.09, 0.03,
    0.27, 0.26,  0.6,
    0.01, 0.47, 0.99,
    
    0.58, 0.04, 0.94,
    0.57, 0.09, 0.03,
    0.27, 0.26,  0.6,
    0.01, 0.47, 0.99
  ]

async function begin(){
    const args = process.argv.slice();
    let experimentType = args[2]

    if(experimentType == 'interactions'){
        console.log('interactions')
        await interactions()
    }else if(experimentType == 'static'){
        console.log('static')
        await static()
    }else if(experimentType == 'linenum'){
        console.log('linenum')
        await linenum()
    }else if(experimentType == 'tsnum'){
        console.log('tsnum')
        await tsnum()
    }
    
    else{
        await test()
    }
}




let isMergeTable = true
let isTreeCache = true
let isParallel = true
let isMemLimit = false
let timelimit = 100 //100s
let procesStartTime = 0




// 使用profile函数计算并打印slowFunction的执行时间
profile( begin);



