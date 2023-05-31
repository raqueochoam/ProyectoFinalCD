import { Component, OnInit } from '@angular/core';
import { ModalController } from '@ionic/angular';

@Component({
  selector: 'app-chat-admin-modal',
  templateUrl: './chat-admin-modal.component.html',
  styleUrls: ['./chat-admin-modal.component.scss'],
})
export class ChatAdminModalComponent  implements OnInit {

  constructor(private modalController: ModalController) { }

  ngOnInit() {}

  close() {
    return this.modalController.dismiss('cancel');
  }

}
