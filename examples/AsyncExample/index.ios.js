/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 */
'use strict';
import React, {
  AppRegistry,
  Component,
  StyleSheet,
  Text,
  View,
  TouchableHighlight
} from 'react-native';

var Realm = require('realm');

var TestObjectSchema = {
  name: 'TestObject',
  properties: { string : 'string' }
}

class AsyncExample extends Component {
  constructor(props) {
    super(props); 
    this.state = {
      text: "",
    };
  }

  async runAsyncQuery() {
    this.setState({text: "Creating " + count + " objects."});
    var realm = new Realm({schema: [TestObjectSchema]});
    var count = 1;
    realm.write(() => {
      for (var i = 0; i < count; i++) {
        realm.create('TestObject', {string: "" + i});
      }
    });

    this.setState({text: "Querying."});

    realm.objects('TestObject').filteredSnapshot("string contains '1' || string contains '2'", (results) => {
        console.log("Got query");
        this.setState({text: "Recieved " + results.length + " query results."});
    });
  }
  
  render() {
    return (
      <View style={styles.container}>
          <Text style={styles.button} onPress={this.runAsyncQuery.bind(this)}>
              Tap to Run Async Test
          </Text>
          <Text>{this.state.text}</Text>
          <Text style={styles.instructions}>
              Press Cmd+R to reload,{'\n'}
              Cmd+D or shake for dev menu
          </Text>
      </View>
    );
  }
}

const styles = StyleSheet.create({
    container: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#F5FCFF',
    },
    button: {
        borderColor: '#cccccc',
        borderRadius: 4,
        borderWidth: 1,
        fontSize: 20,
        textAlign: 'center',
        margin: 20,
        padding: 10,
    },
    instructions: {
        textAlign: 'center',
        color: '#333333',
        marginBottom: 5,
    },
});

AppRegistry.registerComponent('AsyncExample', () => AsyncExample);
