from node import Node

class Queue():
    def __init__(self):
        self.head = None
        self.tail = None
        
    def enqueue(self, item):
        node = Node(item)
        
        if self.head == None:
            self.head = node
            self.tail = node
            return
        
        self.tail.next = node
        self.tail = node
        return
        
    def dequeue(self):
        if self.isEmpty():
            return None
            
        dequeued = self.head
        self.head = self.head.next
        
        if self.head == None:
            self.tail = None
            
        return dequeued.value
    
    def isEmpty(self):
        if self.head == None:
            return True
        return False 
        
    def length(self):
        n = 0
        
        if self.isEmpty():
            return n
        
        actual = self.head
        
        while actual != None:
            n += 1
            actual = actual.next
            
        return n
    
    def front(self):
        if self.isEmpty():
            return None
        return self.head.value

if __name__ == "__main__":
    queue1 = Queue()
    queue1.enqueue(1)
    queue1.enqueue(2)
    print(queue1.dequeue())  
    print(queue1.isEmpty())
    print(queue1.length())
    print(queue1.front())   