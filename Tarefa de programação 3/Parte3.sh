#!/bin/bash

enable #entra no modo privilegiado
config terminal #acessa as configurações do switch
vlan 10 #acessa a interface de vlan de id 10
name professores #define o nome dessa vlan
vlan 20 #acessa a interface de vlan de id 20
name alunos #define o nome dessa vlan
exit #sai da interface de vlan
interface range fa 0/1-2 #acessa a interface das portas de 1 à 2
switchport mode access #entra em modo de acesso
switchport access vlan 10 #define a vlan que essas portas pertencem
exit #sai da interface de portas
interface range fa 0/3-4 #acessa a interface das portas de 3 à 4
switchport mode access #entra em modo de acesso
switchport access vlan 20 #define a vlan que essas portas pertencem
exit #sai da interface de portas
ip routing #ativa a configuração de roteador do switch L3
interface vlan 10 #acessa a interface logica da vlan de id 10
ip add 192.168.10.1 255.255.255.0 #define o ip da interface logica  
no shutdown #habilia a conexão da porta
exit #sai da interface logica
interface vlan 20 #acessa a interface logica da vlan de id 20
ip add 192.168.20.1 255.255.255.0 #define o ip da interface logica
no shutdown #habilia a conexão da porta
exit #sai da interface logica
