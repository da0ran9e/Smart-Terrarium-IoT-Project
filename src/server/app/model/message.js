class Message {
    constructor(sender, recipient, content) {
      this.sender = sender;
      this.recipient = recipient;
      this.content = content;
      this.timestamp = new Date();
    }
  }
  
  module.exports = Message;